
#include "physics/PhysicsSystem.h"
#include "core/World.h"

#include <bullet/btBulletDynamicsCommon.h>
#include <bullet/BulletDynamics/Dynamics/btRigidBody.h>
#include "physics/Collider.h"
#include "physics/CollisionObject.h"
#include "physics/RigidBody.h"
#include "physics/StaticBody.h"
#include "physics/KinematicBody.h"

#include "physics/BulletUtil.h"

class TransformMotionState : public btMotionState
{
public:
    std::weak_ptr<CollisionObject> target;
    std::map<std::weak_ptr<CollisionObject>, PhysicsSystem::CollisionObjectData, std::owner_less<>>* collisionObjects;
    btRigidBody* body = nullptr;

    TransformMotionState(std::shared_ptr<CollisionObject> _target, 
        std::map<std::weak_ptr<CollisionObject>,
            PhysicsSystem::CollisionObjectData, std::owner_less<>>* _collisionObjects)
        : target(_target), collisionObjects(_collisionObjects)
    { }

    virtual void getWorldTransform(btTransform& worldTransform) const override
    {
        std::shared_ptr<CollisionObject> obj = target.lock();
        std::shared_ptr<Transform> transform = obj->getTransform();
        worldTransform = convert(transform ? transform->getGlobalTransform() : TransformData());
    }

    virtual void setWorldTransform(const btTransform& worldTransform) override
    {
        std::shared_ptr<CollisionObject> obj = target.lock();
        std::shared_ptr<Transform> transform = obj->getTransform();
        if(transform) {
            TransformData td = convert(body ? body->getWorldTransform() : worldTransform);
            td.scale = transform->getGlobalTransform().scale;
            transform->setGlobalTransform(td);
            collisionObjects->find(obj)->second.updateId = transform->sumUpdates();
        }
    }
};

PhysicsSystem::~PhysicsSystem()
{
    if(physicsWorld) { delete physicsWorld; }
    if(configuration) { delete configuration; }
    if(dispatcher) { delete dispatcher; }
    if(broadphase) { delete broadphase; }
    if(solver) { delete solver; }

    for(auto& p : collisionObjects) {
        cleanUpCollisionObject(p.second);
    }
}

void PhysicsSystem::setGravity(const glm::vec3& _gravity)
{
    gravity = _gravity;
    if(physicsWorld) {
        physicsWorld->setGravity(convert(gravity));
    }
}

void PhysicsSystem::init()
{
    configuration = new btDefaultCollisionConfiguration();
    dispatcher = new btCollisionDispatcher(configuration);
    broadphase = new btDbvtBroadphase();
    solver = new btSequentialImpulseConstraintSolver();
    physicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, configuration);
    physicsWorld->setGravity(convert(gravity));
}

void PhysicsSystem::gameplayTick(float delta)
{
    std::set<std::shared_ptr<CollisionObject>> validBodies;
    // Copy component data to bullet DSs.
    for(auto it = collisionObjects.begin(); it != collisionObjects.end(); ) {
        std::shared_ptr<CollisionObject> col = it->first.lock();
        if(!col) {
            cleanUpCollisionObject(it->second);
            collisionObjects.erase(it++);
        } else {
            validBodies.insert(col);
            updateCollidersOfObject(col, it->second);
            updateStateOfObject(col, it->second);
            ++it;
        }
    }
    // Look through all body components and setup any new bodies.
    Query<std::shared_ptr<CollisionObject>> allBodies = getWorld()->queryComponents()
        .filter([](std::shared_ptr<Component> ptr){
            return ptr->getTypeId() == get_id(RigidBody)
                || ptr->getTypeId() == get_id(StaticBody)
                || ptr->getTypeId() == get_id(KinematicBody);
        })
        .cast_ptr<CollisionObject>()
        .filter([](std::shared_ptr<CollisionObject> ptr) {
            return ptr->colliders.size() > 0;
        });
    for(std::shared_ptr<CollisionObject> body : allBodies) {
        auto p = validBodies.insert(body);
        if(!p.second) {
            continue;
        }
        setUpCollisionObject(body);
    }

    // Step the simulation one frame.
    physicsWorld->stepSimulation(delta, 0);

    int manifolds = dispatcher->getNumManifolds();
    for(int i = 0; i < manifolds; i++) {
        btPersistentManifold* manifold = dispatcher->getManifoldByIndexInternal(i);
        const btCollisionObject* btObjectA = static_cast<const btCollisionObject*>(manifold->getBody0());
        const btCollisionObject* btObjectB = static_cast<const btCollisionObject*>(manifold->getBody1());

        CollisionObject* objectA = static_cast<CollisionObject*>(btObjectA->getUserPointer());
        CollisionObject* objectB = static_cast<CollisionObject*>(btObjectB->getUserPointer());

        CollisionObject::Hit* hitA = objectA->findOrCreateHit(
            std::static_pointer_cast<CollisionObject>(objectB->shared_from_this()));
        CollisionObject::Hit* hitB = objectB->findOrCreateHit(
            std::static_pointer_cast<CollisionObject>(objectA->shared_from_this()));
        
        int contacts = manifold->getNumContacts();
        for(int j = 0; j < contacts; j++) {
            const btManifoldPoint& contact = manifold->getContactPoint(j);

            hitA->contacts.push_back(CollisionObject::Contact(
                convert(contact.getPositionWorldOnA()),
                convert(contact.m_localPointA),
                -convert(contact.m_normalWorldOnB)));
            hitB->contacts.push_back(CollisionObject::Contact(
                convert(contact.getPositionWorldOnB()),
                convert(contact.m_localPointB),
                convert(contact.m_normalWorldOnB)));
        }
    }
}

void PhysicsSystem::cleanUpCollisionObject(PhysicsSystem::CollisionObjectData& body)
{
    delete body.collisionObject;
    delete body.compoundShape;
    delete body.motionState;
    for(auto p : body.shapeMap) {
        delete p.second.first;
    }
}

void PhysicsSystem::setUpCollisionObject(std::shared_ptr<CollisionObject>& bodyComponent)
{
    CollisionObjectData data;
    data.compoundShape = new btCompoundShape();
    std::shared_ptr<Transform> bodyTransform = bodyComponent->getTransform();
    for(std::shared_ptr<Collider>& collider : bodyComponent->getColliders())
    {
        btCollisionShape* shape = collider->constructShape();
        std::shared_ptr<Transform> transform = collider->getTransform();
        TransformData td = transform->getTransformRelativeTo(bodyComponent->getTransform());
        uint updateId = transform->sumUpdatesRelativeTo(bodyTransform);
        data.compoundShape->addChildShape(convert(td), shape);
        data.shapeMap.insert(std::make_pair(collider, std::make_pair(shape, updateId)));
        collider->shapeUpdated = false;
    }
    TransformMotionState* tms = new TransformMotionState(bodyComponent, &collisionObjects);
    data.updateId = bodyTransform->sumUpdates();
    data.motionState = tms;
    data.collisionObject = bodyComponent->constructObject(data.compoundShape, data.motionState);
    data.collisionObject->setUserPointer(bodyComponent.get());
    bodyComponent->body = data.collisionObject;
    bodyComponent->hits.clear();
    btRigidBody* asRB = btRigidBody::upcast(data.collisionObject);
    tms->body = asRB;
    if(asRB) {
        physicsWorld->addRigidBody(asRB);
        data.type = CollisionObjectData::RigidBody;
    } else {
        physicsWorld->addCollisionObject(data.collisionObject);
        data.type = CollisionObjectData::Generic;
    }
    collisionObjects.insert(std::make_pair(bodyComponent, data));
}

std::map<btCollisionShape*, int> getChildMap(btCompoundShape* shape)
{
    btCompoundShapeChild* children = shape->getChildList();
    int childCount = shape->getNumChildShapes();
    std::map<btCollisionShape*, int> childMap;
    for(int i = 0; i < childCount; i++) {
        childMap.insert(std::make_pair(children[i].m_childShape, i));
    }
    return childMap;
}

void PhysicsSystem::updateCollidersOfObject(std::shared_ptr<CollisionObject>& bodyComponent,
    PhysicsSystem::CollisionObjectData& bodyData)
{
    std::shared_ptr<Transform> bodyTransform = bodyComponent->getTransform();
    std::map<btCollisionShape*, int> childMap = getChildMap(bodyData.compoundShape);
    bool shapeUpdated = false;
    std::set<std::shared_ptr<Collider>> colliders;
    // Go through each collider and ensure it exists and isn't updated.
    for(std::shared_ptr<Collider>& collider : bodyComponent->getColliders()) {
        if(!collider) {
            continue;
        }
        colliders.insert(collider);
        std::shared_ptr<Transform> transform = collider->getTransform();
        auto it = bodyData.shapeMap.find(collider);
        TransformData td = transform->getTransformRelativeTo(bodyComponent->getTransform());
        uint updateId = transform->sumUpdatesRelativeTo(bodyTransform);

        // Check if this collider needs an update.
        bool colliderUpdated = it == bodyData.shapeMap.end() || collider->shapeUpdated || it->second.second != updateId;
        if(!colliderUpdated) { // Move on if it doesn't
            continue;
        }
        // The first collider that is updated needs to remove the object from the world.
        if(!shapeUpdated) {
            shapeUpdated = true;
            if(bodyData.type == CollisionObjectData::RigidBody) {
                physicsWorld->removeRigidBody(btRigidBody::upcast(bodyData.collisionObject));
            } else {
                physicsWorld->removeCollisionObject(bodyData.collisionObject);
            }
        }

        // If the collider is not part of the rigidbody, we need to create its shape.
        if(it == bodyData.shapeMap.end()) {
            btCollisionShape* shape = collider->constructShape();
            shape->setLocalScaling(convert(td.scale));
            bodyData.compoundShape->addChildShape(convert(td), shape);
            bodyData.shapeMap.insert(std::make_pair(collider, std::make_pair(shape, updateId)));
            // Update the child map.
            childMap = getChildMap(bodyData.compoundShape);
        } else if(collider->shapeUpdated) {
            // Otherwise, if the shape has been updated, we need to rebuild the collider.
            bodyData.compoundShape->removeChildShape(it->second.first);
            delete it->second.first;
            btCollisionShape* shape = collider->constructShape();
            shape->setLocalScaling(convert(td.scale));
            bodyData.compoundShape->addChildShape(convert(td), shape);
            it->second.first = shape;
            it->second.second = updateId;
            // Reset the updated flag.
            collider->shapeUpdated = false;
            // Update the child map.
            childMap = getChildMap(bodyData.compoundShape);
        } else {
            // This is if the transform was updated.
            bodyData.compoundShape->updateChildTransform(childMap[it->second.first], convert(td), false);
            it->second.first->setLocalScaling(convert(td.scale));
        }
    }
    
    std::set<std::weak_ptr<Collider>, std::owner_less<>> eraseTgts;
    for(auto p : bodyData.shapeMap) {
        if(colliders.find(p.first.lock()) == colliders.end()) {
            eraseTgts.insert(p.first);
        }
    }
    for(std::weak_ptr<Collider> collider : eraseTgts) {
        if(!shapeUpdated) {
            shapeUpdated = true;
            if(bodyData.type == CollisionObjectData::RigidBody) {
                physicsWorld->removeRigidBody(btRigidBody::upcast(bodyData.collisionObject));
            } else {
                physicsWorld->removeCollisionObject(bodyData.collisionObject);
            }
        }

        auto it = bodyData.shapeMap.find(collider);
        bodyData.compoundShape->removeChildShape(it->second.first);
        delete it->second.first;
        bodyData.shapeMap.erase(it);
    }

    if(shapeUpdated) {
        bodyData.compoundShape->recalculateLocalAabb();
        if(bodyData.type == CollisionObjectData::RigidBody) {
            btRigidBody* rb = btRigidBody::upcast(bodyData.collisionObject);
            rb->updateInertiaTensor();
            physicsWorld->addRigidBody(rb);
        } else {
            physicsWorld->addCollisionObject(bodyData.collisionObject);
        }
    }
}

void PhysicsSystem::updateStateOfObject(std::shared_ptr<CollisionObject>& bodyComponent, CollisionObjectData& bodyData)
{
    // Clear any hit info.
    bodyComponent->hits.clear();

    std::shared_ptr<Transform> transform = bodyComponent->getTransform();
    if(transform->sumUpdates() == bodyData.updateId) {
        return;
    }
    TransformData globalTransform = transform->getGlobalTransform();
    bodyData.compoundShape->setLocalScaling(convert(globalTransform.scale));
    if(!(bodyData.collisionObject->getCollisionFlags() & btCollisionObject::CF_KINEMATIC_OBJECT))
    {
        bodyData.collisionObject->setWorldTransform(convert(globalTransform));
    }
}
