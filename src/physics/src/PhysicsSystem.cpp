
#include "physics/PhysicsSystem.h"
#include "core/World.h"

#include <bullet/btBulletDynamicsCommon.h>
#include <bullet/BulletDynamics/Dynamics/btRigidBody.h>
#include <bullet/BulletCollision/CollisionDispatch/btGhostObject.h>
#include <bullet/BulletCollision/NarrowPhaseCollision/btRaycastCallback.h>
#include "physics/Collider.h"
#include "physics/CollisionObject.h"
#include "physics/RigidBody.h"
#include "physics/StaticBody.h"
#include "physics/KinematicBody.h"
#include "physics/Trigger.h"

#include "physics/BulletUtil.h"

// PhysicsSystemQueries.cpp contains the rest of the implementation for PhysicsSystem

class TransformMotionState : public btMotionState
{
public:
    weak_ptr<CollisionObject> target;
    map<weak_ptr<CollisionObject>, PhysicsSystem::CollisionObjectData, owner_less<>>* collisionObjects;
    btRigidBody* body = nullptr;

    TransformMotionState(shared_ptr<CollisionObject> _target, 
        map<weak_ptr<CollisionObject>,
            PhysicsSystem::CollisionObjectData, owner_less<>>* _collisionObjects)
        : target(_target), collisionObjects(_collisionObjects)
    { }

    virtual void getWorldTransform(btTransform& worldTransform) const override
    {
        shared_ptr<CollisionObject> obj = target.lock();
        shared_ptr<Transform> transform = obj->getTransform();
        worldTransform = convert(transform ? transform->getGlobalTransform() : TransformData());
    }

    virtual void setWorldTransform(const btTransform& worldTransform) override
    {
        shared_ptr<CollisionObject> obj = target.lock();
        shared_ptr<Transform> transform = obj->getTransform();
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
    if(triggerCallback) { delete triggerCallback; }

    physicsWorld = nullptr;

    for(auto& p : collisionObjects) {
        cleanUpCollisionObject(p.second);
    }
}

void PhysicsSystem::setGravity(const vec3& _gravity)
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
    triggerCallback = new btGhostPairCallback();
    physicsWorld->getPairCache()->setInternalGhostPairCallback(triggerCallback);
    physicsWorld->setGravity(convert(gravity));
}

void PhysicsSystem::gameplayTick(float delta)
{
    hash_set<shared_ptr<CollisionObject>> validBodies;
    // Copy component data to bullet DSs.
    for(auto it = collisionObjects.begin(); it != collisionObjects.end(); ) {
        shared_ptr<CollisionObject> col = it->first.lock();
        if(!col) {
            reverseObjects.erase(it->second.collisionObject);
            cleanUpCollisionObject(it->second);
            collisionObjects.erase(it++);
        } else {
            validBodies.insert(col);
            updateCollidersOfObject(col, it->second);
            updateStateOfObject(col, it->second);
            ++it;
        }
    }
    hash_set<CollisionObject*> invalidBodies;
    // Look through all body components and setup any new bodies.
    Query<shared_ptr<CollisionObject>> allBodies = (
        getWorld()->queryComponents(get_id(RigidBody))
        | getWorld()->queryComponents(get_id(StaticBody))
        | getWorld()->queryComponents(get_id(KinematicBody))
        | getWorld()->queryComponents(get_id(Trigger))
        )
        .cast_ptr<CollisionObject>()
        .filter([](shared_ptr<CollisionObject> ptr) {
            return ptr->colliders.size() > 0;
        });
    for(shared_ptr<CollisionObject> body : allBodies) {
        auto p = validBodies.insert(body);
        if(!p.second) {
            continue;
        }
        setUpCollisionObject(body);
    }

    // Step the simulation one frame.
    physicsWorld->stepSimulation(delta, 0);

    // Go through all triggers we know about, clear their overlaps, and copy over their new overlaps.
    for(auto& pair : collisionObjects) {
        shared_ptr<CollisionObject> col = pair.first.lock();
        if(!col || col->getTypeId() != get_id(Trigger)) {
            continue;
        }
        shared_ptr<Trigger> trigger = static_pointer_cast<Trigger>(col);
        btGhostObject* btTrigger = static_cast<btGhostObject*>(pair.second.collisionObject);
        trigger->overlaps.clear();
        int overlaps = btTrigger->getNumOverlappingObjects();
        trigger->overlaps.reserve(overlaps);
        for(int i = 0; i < overlaps; i++) {
            auto jt = reverseObjects.find(btTrigger->getOverlappingObject(i));
            if(jt == reverseObjects.end()) {
                throw "Really not sure what heppened. Overlapped with an unknown collision object.";
            }
            trigger->overlaps.push_back(jt->second);
        }
    }

    int manifolds = dispatcher->getNumManifolds();
    for(int i = 0; i < manifolds; i++) {
        btPersistentManifold* manifold = dispatcher->getManifoldByIndexInternal(i);
        const btCollisionObject* btObjectA = static_cast<const btCollisionObject*>(manifold->getBody0());
        const btCollisionObject* btObjectB = static_cast<const btCollisionObject*>(manifold->getBody1());

        CollisionObject* objectA = static_cast<CollisionObject*>(btObjectA->getUserPointer());
        CollisionObject* objectB = static_cast<CollisionObject*>(btObjectB->getUserPointer());

        CollisionObject::Hit* hitA = objectA->findOrCreateHit(
            static_pointer_cast<CollisionObject>(objectB->shared_from_this()));
        CollisionObject::Hit* hitB = objectB->findOrCreateHit(
            static_pointer_cast<CollisionObject>(objectA->shared_from_this()));
        
        int contacts = manifold->getNumContacts();
        for(int j = 0; j < contacts; j++) {
            const btManifoldPoint& contact = manifold->getContactPoint(j);

            // TODO: Figure out how much force was applied by the contact.

            hitA->contacts.push_back(CollisionObject::Contact(
                convert(contact.getPositionWorldOnA()),
                convert(contact.m_localPointA),
                convert(contact.m_normalWorldOnB)));
            hitB->contacts.push_back(CollisionObject::Contact(
                convert(contact.getPositionWorldOnB()),
                convert(contact.m_localPointB),
                -convert(contact.m_normalWorldOnB)));
        }
    }
}

void PhysicsSystem::cleanUpCollisionObject(PhysicsSystem::CollisionObjectData& body)
{
    if(physicsWorld) {
        removeBody(body);
    }
    delete body.collisionObject;
    delete body.compoundShape;
    delete body.motionState;
    for(auto p : body.shapeMap) {
        delete p.second.first;
    }
}

void PhysicsSystem::setUpCollisionObject(shared_ptr<CollisionObject>& bodyComponent)
{
    CollisionObjectData data; 
    data.compoundShape = new btCompoundShape();
    shared_ptr<Transform> bodyTransform = bodyComponent->getTransform();
    for(shared_ptr<Collider>& collider : bodyComponent->getColliders())
    {
        btCollisionShape* shape = collider->constructShape();
        shared_ptr<Transform> transform = collider->getTransform();
        uint updateId = transform->sumUpdatesRelativeTo(bodyTransform);
        if(shape) {
            TransformData td = transform->getTransformRelativeTo(bodyComponent->getTransform());
            shape->setLocalScaling(convert(td.scale));
            data.compoundShape->addChildShape(convert(td), shape);
        }
        data.shapeMap.insert(make_pair(collider, make_pair(shape, updateId)));
        collider->shapeUpdated = false;
    }
    // No point in constructing the motion state if we won't use it.
    TransformMotionState* tms = bodyComponent->getTypeId() == get_id(Trigger) ? nullptr
        : new TransformMotionState(bodyComponent, &collisionObjects);
    data.updateId = bodyTransform->sumUpdates();
    data.motionState = tms;
    TransformData bodyTD = bodyTransform->getGlobalTransform();
    data.compoundShape->setLocalScaling(convert(bodyTD.scale));
    data.collisionObject = bodyComponent->constructObject(data.compoundShape, data.motionState);
    if(bodyComponent->getTypeId() == get_id(Trigger)) {
        data.collisionObject->setWorldTransform(convert(bodyTD));
    }
    data.collisionObject->setUserPointer(bodyComponent.get());
    bodyComponent->body = data.collisionObject;
    bodyComponent->hits.clear();
    btRigidBody* asRB = btRigidBody::upcast(data.collisionObject);
    if(tms) {
        tms->body = asRB;
    }
    if(asRB) {
        data.type = CollisionObjectData::RigidBody;
    } else {
        data.type = CollisionObjectData::Generic;
    }
    addBody(data);

    collisionObjects.insert(make_pair(bodyComponent, data));
    reverseObjects.insert(make_pair(data.collisionObject, bodyComponent));
}

hash_map<btCollisionShape*, int> getChildMap(btCompoundShape* shape)
{
    btCompoundShapeChild* children = shape->getChildList();
    int childCount = shape->getNumChildShapes();
    hash_map<btCollisionShape*, int> childMap;
    for(int i = 0; i < childCount; i++) {
        childMap.insert(make_pair(children[i].m_childShape, i));
    }
    return childMap;
}

void PhysicsSystem::updateCollidersOfObject(shared_ptr<CollisionObject>& bodyComponent,
    PhysicsSystem::CollisionObjectData& bodyData)
{
    shared_ptr<Transform> bodyTransform = bodyComponent->getTransform();
    hash_map<btCollisionShape*, int> childMap = getChildMap(bodyData.compoundShape);
    bool shapeUpdated = false;
    hash_set<shared_ptr<Collider>> colliders;
    // Go through each collider and ensure it exists and isn't updated.
    for(shared_ptr<Collider>& collider : bodyComponent->getColliders()) {
        if(!collider) {
            continue;
        }
        colliders.insert(collider);
        shared_ptr<Transform> transform = collider->getTransform();
        auto it = bodyData.shapeMap.find(collider);
        TransformData td = transform->getTransformRelativeTo(bodyComponent->getTransform());
        uint updateId = transform->sumUpdatesRelativeTo(bodyTransform);

        // Check if this collider needs an update.
        bool colliderUpdated = it == bodyData.shapeMap.end() || !it->second.first
            || collider->shapeUpdated || it->second.second != updateId;
        if(!colliderUpdated) { // Move on if it doesn't
            continue;
        }

        // We need to handle the special case of the collision shape being null.
        // We only want to update the shape if the new shape is not null.
        if(it != bodyData.shapeMap.end() && !it->second.first) {
            btCollisionShape* shape = collider->constructShape();
            if(shape) {
                // The first collider that is updated needs to remove the object from the world.
                if(!shapeUpdated) {
                    shapeUpdated = true;
                    removeBody(bodyData);
                }

                shape->setLocalScaling(convert(td.scale));
                bodyData.compoundShape->addChildShape(convert(td), shape);

                it->second.first = shape;
                it->second.second = updateId;
                // Update the child map.
                childMap = getChildMap(bodyData.compoundShape);
            }
        }
        
        // The first collider that is updated needs to remove the object from the world.
        if(!shapeUpdated) {
            shapeUpdated = true;
            removeBody(bodyData);
        }

        // If the collider is not part of the rigidbody, we need to create its shape.
        if(it == bodyData.shapeMap.end()) {
            btCollisionShape* shape = collider->constructShape();
            if(shape) {
                shape->setLocalScaling(convert(td.scale));
                bodyData.compoundShape->addChildShape(convert(td), shape);
            }
            if(it == bodyData.shapeMap.end()) {
                bodyData.shapeMap.insert(make_pair(collider, make_pair(shape, updateId)));
            } else {
                it->second.first = shape;
                it->second.second = updateId;
            }
            // Update the child map.
            childMap = getChildMap(bodyData.compoundShape);
        } else if(collider->shapeUpdated) {
            // Otherwise, if the shape has been updated, we need to rebuild the collider.
            bodyData.compoundShape->removeChildShape(it->second.first);
            delete it->second.first;
            btCollisionShape* shape = collider->constructShape();
            if(shape) {
                shape->setLocalScaling(convert(td.scale));
                bodyData.compoundShape->addChildShape(convert(td), shape);
            }
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
    
    set<weak_ptr<Collider>, owner_less<>> eraseTgts;
    for(auto p : bodyData.shapeMap) {
        if(colliders.find(p.first.lock()) == colliders.end()) {
            eraseTgts.insert(p.first);
        }
    }
    for(weak_ptr<Collider> collider : eraseTgts) {
        if(!shapeUpdated) {
            shapeUpdated = true;
            removeBody(bodyData);
        }

        auto it = bodyData.shapeMap.find(collider);
        bodyData.compoundShape->removeChildShape(it->second.first);
        delete it->second.first;
        bodyData.shapeMap.erase(it);
    }

    if(shapeUpdated) {
        bodyData.compoundShape->recalculateLocalAabb();
        btRigidBody* rb = btRigidBody::upcast(bodyData.collisionObject);
        rb->updateInertiaTensor();
        addBody(bodyData);
    }
}

void PhysicsSystem::updateStateOfObject(shared_ptr<CollisionObject>& bodyComponent, CollisionObjectData& bodyData)
{
    // Clear any hit info.
    bodyComponent->hits.clear();

    shared_ptr<Transform> transform = bodyComponent->getTransform();
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

void PhysicsSystem::addBody(CollisionObjectData& body)
{
    if(body.type == CollisionObjectData::RigidBody) {
        physicsWorld->addRigidBody(btRigidBody::upcast(body.collisionObject));
    } else {
        physicsWorld->addCollisionObject(body.collisionObject);
    }
}

void PhysicsSystem::removeBody(CollisionObjectData& body)
{
    if(body.type == CollisionObjectData::RigidBody) {
        physicsWorld->removeRigidBody(btRigidBody::upcast(body.collisionObject));
    } else {
        physicsWorld->removeCollisionObject(body.collisionObject);
    }
}
