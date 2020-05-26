
#include "physics/PhysicsSystem.h"
#include "core/World.h"

#include <bullet/btBulletDynamicsCommon.h>
#include <bullet/BulletDynamics/Dynamics/btRigidBody.h>
#include "physics/Collider.h"
#include "physics/CollisionObject.h"

#include "physics/BulletUtil.h"

class TransformMotionState : public btMotionState
{
public:
    std::weak_ptr<CollisionObject> target;
    btRigidBody* body = nullptr;

    TransformMotionState(std::shared_ptr<CollisionObject> _target)
    {
        target = _target;
    }

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
            return ptr->getTypeId() == RIGIDBODY_ID
                || ptr->getTypeId() == STATICBODY_ID;
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
}

void PhysicsSystem::cleanUpCollisionObject(PhysicsSystem::CollisionObjectData& body)
{
    delete body.collisionObject;
    delete body.compoundShape;
    delete body.motionState;
    for(auto p : body.shapeMap) {
        delete p.second;
    }
}

void PhysicsSystem::setUpCollisionObject(std::shared_ptr<CollisionObject>& bodyComponent)
{
    CollisionObjectData data;
    data.compoundShape = new btCompoundShape();
    for(std::shared_ptr<Collider>& collider : bodyComponent->getColliders())
    {
        if(!collider) {
            continue;
        }
        btCollisionShape* shape = collider->constructShape();
        TransformData td = collider->getTransform()->getTransformRelativeTo(bodyComponent->getTransform());
        data.compoundShape->addChildShape(convert(td), shape);
        data.shapeMap.insert(std::make_pair(collider, shape));
        collider->shapeUpdated = false;
    }
    TransformMotionState* tms = new TransformMotionState(bodyComponent);
    data.motionState = tms;
    data.collisionObject = bodyComponent->constructObject(data.compoundShape, data.motionState);
    bodyComponent->body = data.collisionObject;
    btRigidBody* asRB = btRigidBody::upcast(data.collisionObject);
    tms->body = asRB;
    if(asRB) {
        physicsWorld->addRigidBody(asRB);
    } else {
        physicsWorld->addCollisionObject(data.collisionObject);
    }
    collisionObjects.insert(std::make_pair(bodyComponent, data));
}

void PhysicsSystem::updateCollidersOfObject(std::shared_ptr<CollisionObject>& bodyComponent,
    PhysicsSystem::CollisionObjectData& bodyData)
{
    btCompoundShapeChild* children = bodyData.compoundShape->getChildList();
    int childCount = bodyData.compoundShape->getNumChildShapes();
    std::map<btCollisionShape*, int> childMap;
    for(int i = 0; i < childCount; i++) {
        childMap.insert(std::make_pair(children[i].m_childShape, i));
    }

    std::set<Collider*> colliders;
    // Go through each collider and ensure it exists and isn't updated.
    for(std::shared_ptr<Collider>& collider : bodyComponent->getColliders()) {
        if(!collider) {
            continue;
        }
        colliders.insert(collider.get());
        auto it = bodyData.shapeMap.find(collider);
        TransformData td = collider->getTransform()->getTransformRelativeTo(bodyComponent->getTransform());
        // If the collider is not part of the rigidbody, we need to create its shape.
        if(it == bodyData.shapeMap.end()) {
            btCollisionShape* shape = collider->constructShape();
            shape->setLocalScaling(convert(td.scale));
            bodyData.compoundShape->addChildShape(convert(td), shape);
            bodyData.shapeMap.insert(std::make_pair(collider, shape));
        } else if(collider->shapeUpdated) {
            // Otherwise, if the shape has been updated, we need to rebuild the collider.
            bodyData.compoundShape->removeChildShape(it->second);
            delete it->second;
            btCollisionShape* shape = collider->constructShape();
            shape->setLocalScaling(convert(td.scale));
            bodyData.compoundShape->addChildShape(convert(td), shape);
            it->second = shape;
            // Reset the updated flag.
            collider->shapeUpdated = false;
        } else {
            bodyData.compoundShape->updateChildTransform(childMap[it->second], convert(td), false);
            it->second->setLocalScaling(convert(td.scale));
        }
    }
    
    std::set<std::weak_ptr<Collider>, std::owner_less<>> eraseTgts;
    for(auto p : bodyData.shapeMap) {
        if(colliders.find(p.first.lock().get()) == colliders.end()) {
            eraseTgts.insert(p.first);
        }
    }
    for(std::weak_ptr<Collider> collider : eraseTgts) {
        auto it = bodyData.shapeMap.find(collider);
        bodyData.compoundShape->removeChildShape(it->second);
        delete it->second;
        bodyData.shapeMap.erase(it);
    }

}

void PhysicsSystem::updateStateOfObject(std::shared_ptr<CollisionObject>& bodyComponent, CollisionObjectData& bodyData)
{
    TransformData globalTransform = bodyComponent->getTransform()->getGlobalTransform();
    bodyData.compoundShape->setLocalScaling(convert(globalTransform.scale));
    if(!(bodyData.collisionObject->getCollisionFlags() & btCollisionObject::CF_KINEMATIC_OBJECT))
    {
        bodyData.collisionObject->setWorldTransform(convert(globalTransform));
    }
}
