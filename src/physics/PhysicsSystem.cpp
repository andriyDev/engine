
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
    if(triggerCallback) { delete triggerCallback; }

    physicsWorld = nullptr;

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
    triggerCallback = new btGhostPairCallback();
    physicsWorld->getPairCache()->setInternalGhostPairCallback(triggerCallback);
    physicsWorld->setGravity(convert(gravity));
}

void PhysicsSystem::gameplayTick(float delta)
{
    std::set<std::shared_ptr<CollisionObject>> validBodies;
    // Copy component data to bullet DSs.
    for(auto it = collisionObjects.begin(); it != collisionObjects.end(); ) {
        std::shared_ptr<CollisionObject> col = it->first.lock();
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
    // Look through all body components and setup any new bodies.
    Query<std::shared_ptr<CollisionObject>> allBodies = getWorld()->queryComponents()
        .filter([](std::shared_ptr<Component> ptr){
            return ptr->getTypeId() == get_id(RigidBody)
                || ptr->getTypeId() == get_id(StaticBody)
                || ptr->getTypeId() == get_id(KinematicBody)
                || ptr->getTypeId() == get_id(Trigger);
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

    // Go through all triggers we know about, clear their overlaps, and copy over their new overlaps.
    for(auto& pair : collisionObjects) {
        std::shared_ptr<CollisionObject> col = pair.first.lock();
        if(!col || col->getTypeId() != get_id(Trigger)) {
            continue;
        }
        std::shared_ptr<Trigger> trigger = std::static_pointer_cast<Trigger>(col);
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
            std::static_pointer_cast<CollisionObject>(objectB->shared_from_this()));
        CollisionObject::Hit* hitB = objectB->findOrCreateHit(
            std::static_pointer_cast<CollisionObject>(objectA->shared_from_this()));
        
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

void PhysicsSystem::setUpCollisionObject(std::shared_ptr<CollisionObject>& bodyComponent)
{
    CollisionObjectData data; 
    data.compoundShape = new btCompoundShape();
    std::shared_ptr<Transform> bodyTransform = bodyComponent->getTransform();
    for(std::shared_ptr<Collider>& collider : bodyComponent->getColliders())
    {
        btCollisionShape* shape = collider->constructShape();
        std::shared_ptr<Transform> transform = collider->getTransform();
        uint updateId = transform->sumUpdatesRelativeTo(bodyTransform);
        if(shape) {
            TransformData td = transform->getTransformRelativeTo(bodyComponent->getTransform());
            shape->setLocalScaling(convert(td.scale));
            data.compoundShape->addChildShape(convert(td), shape);
        }
        data.shapeMap.insert(std::make_pair(collider, std::make_pair(shape, updateId)));
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

    collisionObjects.insert(std::make_pair(bodyComponent, data));
    reverseObjects.insert(std::make_pair(data.collisionObject, bodyComponent));
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
                bodyData.shapeMap.insert(std::make_pair(collider, std::make_pair(shape, updateId)));
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
    
    std::set<std::weak_ptr<Collider>, std::owner_less<>> eraseTgts;
    for(auto p : bodyData.shapeMap) {
        if(colliders.find(p.first.lock()) == colliders.end()) {
            eraseTgts.insert(p.first);
        }
    }
    for(std::weak_ptr<Collider> collider : eraseTgts) {
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

struct FilterRaysCallback : public btCollisionWorld::RayResultCallback
{
public:
    btCollisionWorld::RayResultCallback* wrappedCallback;
    const std::map<btCollisionObject*, std::weak_ptr<CollisionObject>>* reverseObjects;
    bool hitTriggers;
    const std::set<std::shared_ptr<CollisionObject>>* ignoredBodies;
    const std::set<std::shared_ptr<Entity>>* ignoreEntities;

    FilterRaysCallback(btCollisionWorld::RayResultCallback* _wrappedCallback,
        const std::map<btCollisionObject*, std::weak_ptr<CollisionObject>>* _reverseObjects)
        : wrappedCallback(_wrappedCallback), reverseObjects(_reverseObjects)
    {
        m_flags = wrappedCallback->m_flags;
        m_collisionFilterMask = wrappedCallback->m_collisionFilterMask;
        m_collisionFilterGroup = wrappedCallback->m_collisionFilterGroup;
    }

    bool hasHit() const {
        return wrappedCallback->hasHit();
    }

    virtual bool needsCollision(btBroadphaseProxy* proxy0) const override {
        return wrappedCallback->needsCollision(proxy0);
    }

    virtual btScalar addSingleResult(btCollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace) override
    {
        auto it = reverseObjects->find(const_cast<btCollisionObject*>(rayResult.m_collisionObject));
        auto CO = it != reverseObjects->end() ? it->second.lock() : nullptr;
        auto EN = CO ? CO->getOwner() : nullptr;
        if((!btGhostObject::upcast(rayResult.m_collisionObject) || hitTriggers)
            && (!CO || ignoredBodies->find(CO) == ignoredBodies->end())
            && (!EN || ignoreEntities->find(EN) == ignoreEntities->end())
        ) { // Do filtering.
            btScalar s = wrappedCallback->addSingleResult(rayResult, normalInWorldSpace);
            m_closestHitFraction = wrappedCallback->m_closestHitFraction;
            m_collisionObject = wrappedCallback->m_collisionObject;
            return s;
        }
        else {
            return wrappedCallback->m_closestHitFraction;
        }
    }
};

RaycastHit PhysicsSystem::rayCast(const glm::vec3& source, const glm::vec3& direction, float range,
    std::shared_ptr<Entity> ignoredEntity, bool hitTriggers) const
{
    std::set<std::shared_ptr<Entity>> ignoredEntities;
    ignoredEntities.insert(ignoredEntity);
    return rayCast(source, direction, range, ignoredEntities,
        std::set<std::shared_ptr<CollisionObject>>(), hitTriggers);
}

RaycastHit PhysicsSystem::rayCast(const glm::vec3& source, const glm::vec3& direction, float range,
    const std::set<std::shared_ptr<Entity>>& ignoredEntities,
    const std::set<std::shared_ptr<CollisionObject>>& ignoredBodies,
    bool hitTriggers) const
{
    RaycastHit result;
    result.valid = false;
    result.point = source + direction * range;
    result.normal = glm::vec3(0,0,0);
    result.obj = std::shared_ptr<CollisionObject>();
    result.fraction = 1;

    if(!physicsWorld) {
        return result;
    }

    btVector3 from = convert(source);
    btVector3 to = convert(result.point); // We already computed the endpoint.

    btCollisionWorld::ClosestRayResultCallback closestRay(from, to);
    closestRay.m_flags |= btTriangleRaycastCallback::kF_KeepUnflippedNormal;

    FilterRaysCallback filter(&closestRay, &reverseObjects);
    filter.ignoredBodies = &ignoredBodies;
    filter.ignoreEntities = &ignoredEntities;
    filter.hitTriggers = hitTriggers;

    physicsWorld->rayTest(from, to, filter);

    if(closestRay.hasHit()) {
        result.valid = true;
        result.point = convert(closestRay.m_hitPointWorld);
        result.obj = reverseObjects.find(const_cast<btCollisionObject*>(closestRay.m_collisionObject))->second.lock();
        result.normal = convert(closestRay.m_hitNormalWorld);
        result.fraction = closestRay.m_closestHitFraction;
    }
    return result;
}
        
std::vector<RaycastHit> PhysicsSystem::rayCastAll(const glm::vec3& source, const glm::vec3& direction, 
    float range, std::shared_ptr<Entity> ignoredEntity, bool hitTriggers) const
{
    std::set<std::shared_ptr<Entity>> ignoredEntities;
    ignoredEntities.insert(ignoredEntity);
    return rayCastAll(source, direction, range, ignoredEntities,
        std::set<std::shared_ptr<CollisionObject>>(), hitTriggers);
}

std::vector<RaycastHit> PhysicsSystem::rayCastAll(const glm::vec3& source, const glm::vec3& direction,
    float range, const std::set<std::shared_ptr<Entity>>& ignoredEntities,
    const std::set<std::shared_ptr<CollisionObject>>& ignoredBodies,
    bool hitTriggers) const
{
    std::vector<RaycastHit> hits;
    if(!physicsWorld) {
        return hits;
    }

    btVector3 from = convert(source);
    btVector3 to = convert(source + direction * range); // We already computed the endpoint.

    btCollisionWorld::AllHitsRayResultCallback allRays(from, to);
    allRays.m_flags |= btTriangleRaycastCallback::kF_KeepUnflippedNormal;

    FilterRaysCallback filter(&allRays, &reverseObjects);
    filter.ignoredBodies = &ignoredBodies;
    filter.ignoreEntities = &ignoredEntities;
    filter.hitTriggers = hitTriggers;

    physicsWorld->rayTest(from, to, filter);

    hits.resize(allRays.m_hitFractions.size());
    for(int i = 0; i < allRays.m_hitFractions.size(); i++) {
        RaycastHit& result = hits[i];
        result.valid = true;
        result.point = convert(allRays.m_hitPointWorld[i]);
        result.obj = reverseObjects.find(const_cast<btCollisionObject*>(allRays.m_collisionObjects[i]))->second.lock();
        result.normal = convert(allRays.m_hitNormalWorld[i]);
        result.fraction = allRays.m_hitFractions[i];
    }
    return hits;
}

struct FilterConvexCallback : public btCollisionWorld::ConvexResultCallback
{
public:
    btCollisionWorld::ConvexResultCallback* wrappedCallback;
    const std::map<btCollisionObject*, std::weak_ptr<CollisionObject>>* reverseObjects;
    bool hitTriggers;
    const std::set<std::shared_ptr<CollisionObject>>* ignoredBodies;
    const std::set<std::shared_ptr<Entity>>* ignoreEntities;

    FilterConvexCallback(btCollisionWorld::ConvexResultCallback* _wrappedCallback,
        const std::map<btCollisionObject*, std::weak_ptr<CollisionObject>>* _reverseObjects)
        : wrappedCallback(_wrappedCallback), reverseObjects(_reverseObjects)
    {
        m_collisionFilterMask = wrappedCallback->m_collisionFilterMask;
        m_collisionFilterGroup = wrappedCallback->m_collisionFilterGroup;
    }

    bool hasHit() const {
        return wrappedCallback->hasHit();
    }

    virtual bool needsCollision(btBroadphaseProxy* proxy0) const override {
        return wrappedCallback->needsCollision(proxy0);
    }

    virtual btScalar addSingleResult(btCollisionWorld::LocalConvexResult& convexResult,
        bool normalInWorldSpace) override
    {
        auto it = reverseObjects->find(const_cast<btCollisionObject*>(convexResult.m_hitCollisionObject));
        auto CO = it != reverseObjects->end() ? it->second.lock() : nullptr;
        auto EN = CO ? CO->getOwner() : nullptr;
        if((!btGhostObject::upcast(convexResult.m_hitCollisionObject) || hitTriggers)
            && (!CO || ignoredBodies->find(CO) == ignoredBodies->end())
            && (!EN || ignoreEntities->find(EN) == ignoreEntities->end())
        ) { // Do filtering.
            btScalar s = wrappedCallback->addSingleResult(convexResult, normalInWorldSpace);
            m_closestHitFraction = wrappedCallback->m_closestHitFraction;
            return s;
        }
        else {
            return wrappedCallback->m_closestHitFraction;
        }
    }
};

struct AllHitsConvexResultCallback : public btCollisionWorld::ConvexResultCallback
{
    btAlignedObjectArray<const btCollisionObject*> m_collisionObjects;

    btAlignedObjectArray<btVector3> m_hitNormalWorld;
    btAlignedObjectArray<btVector3> m_hitPointWorld;
    btAlignedObjectArray<btScalar> m_hitFractions;

    virtual btScalar addSingleResult(btCollisionWorld::LocalConvexResult& convexResult, bool normalInWorldSpace)
    {
        m_collisionObjects.push_back(convexResult.m_hitCollisionObject);
        btVector3 hitNormalWorld;
        if (normalInWorldSpace)
        {
            hitNormalWorld = convexResult.m_hitNormalLocal;
        }
        else
        {
            ///need to transform normal into worldspace
            hitNormalWorld = convexResult.m_hitCollisionObject->getWorldTransform().getBasis()
                * convexResult.m_hitNormalLocal;
        }
        m_hitNormalWorld.push_back(hitNormalWorld);
        m_hitPointWorld.push_back(convexResult.m_hitPointLocal);
        m_hitFractions.push_back(convexResult.m_hitFraction);
        return m_closestHitFraction;
    }
};

RaycastHit PhysicsSystem::shapeCast(const class btConvexShape* shape,
    const glm::vec3& sourcePosition, const glm::quat& sourceRotation,
    const glm::vec3& targetPosition, const glm::quat& targetRotation,
    std::shared_ptr<Entity> ignoredEntity, bool hitTriggers) const
{
    std::set<std::shared_ptr<Entity>> ignoredEntities;
    ignoredEntities.insert(ignoredEntity);
    return shapeCast(shape, sourcePosition, sourceRotation, targetPosition, targetRotation,
        ignoredEntities, std::set<std::shared_ptr<CollisionObject>>(), hitTriggers);
}

RaycastHit PhysicsSystem::shapeCast(const class btConvexShape* shape,
    const glm::vec3& sourcePosition, const glm::quat& sourceRotation,
    const glm::vec3& targetPosition, const glm::quat& targetRotation,
    const std::set<std::shared_ptr<Entity>>& ignoredEntities,
    const std::set<std::shared_ptr<CollisionObject>>& ignoredBodies, bool hitTriggers) const
{
    RaycastHit result;
    result.valid = false;
    result.point = targetPosition;
    result.normal = glm::vec3(0,0,0);
    result.obj = std::shared_ptr<CollisionObject>();
    result.fraction = 1;

    if(!physicsWorld) {
        return result;
    }

    btTransform from = btTransform(convert(sourceRotation), convert(sourcePosition));
    btTransform to = btTransform(convert(targetRotation), convert(targetPosition));

    btCollisionWorld::ClosestConvexResultCallback closestConvex(from.getOrigin(), to.getOrigin());

    FilterConvexCallback filter(&closestConvex, &reverseObjects);
    filter.ignoredBodies = &ignoredBodies;
    filter.ignoreEntities = &ignoredEntities;
    filter.hitTriggers = hitTriggers;

    physicsWorld->convexSweepTest(shape, from, to, filter);

    if(closestConvex.hasHit()) {
        result.valid = true;
        result.point = convert(closestConvex.m_hitPointWorld);
        result.obj = reverseObjects.find(
            const_cast<btCollisionObject*>(closestConvex.m_hitCollisionObject))->second.lock();
        result.normal = convert(closestConvex.m_hitNormalWorld);
        result.fraction = closestConvex.m_closestHitFraction;
    }
    return result;
}
    
std::vector<RaycastHit> PhysicsSystem::shapeCastAll(const class btConvexShape* shape,
    const glm::vec3& sourcePosition, const glm::quat& sourceRotation,
    const glm::vec3& targetPosition, const glm::quat& targetRotation,
    std::shared_ptr<Entity> ignoredEntity, bool hitTriggers) const
{
    std::set<std::shared_ptr<Entity>> ignoredEntities;
    ignoredEntities.insert(ignoredEntity);
    return shapeCastAll(shape, sourcePosition, sourceRotation, targetPosition, targetRotation,
        ignoredEntities, std::set<std::shared_ptr<CollisionObject>>(), hitTriggers);
}

std::vector<RaycastHit> PhysicsSystem::shapeCastAll(const class btConvexShape* shape,
    const glm::vec3& sourcePosition, const glm::quat& sourceRotation,
    const glm::vec3& targetPosition, const glm::quat& targetRotation,
    const std::set<std::shared_ptr<Entity>>& ignoredEntities,
    const std::set<std::shared_ptr<CollisionObject>>& ignoredBodies,
    bool hitTriggers) const
{
    std::vector<RaycastHit> hits;
    if(!physicsWorld) {
        return hits;
    }

    btTransform from = btTransform(convert(sourceRotation), convert(sourcePosition));
    btTransform to = btTransform(convert(targetRotation), convert(targetPosition));

    AllHitsConvexResultCallback allConvex;

    FilterConvexCallback filter(&allConvex, &reverseObjects);
    filter.ignoredBodies = &ignoredBodies;
    filter.ignoreEntities = &ignoredEntities;
    filter.hitTriggers = hitTriggers;

    physicsWorld->convexSweepTest(shape, from, to, filter);

    hits.resize(allConvex.m_hitFractions.size());
    for(int i = 0; i < allConvex.m_hitFractions.size(); i++) {
        RaycastHit& result = hits[i];
        result.valid = true;
        result.point = convert(allConvex.m_hitPointWorld[i]);
        result.obj = reverseObjects.find(
            const_cast<btCollisionObject*>(allConvex.m_collisionObjects[i]))->second.lock();
        result.normal = convert(allConvex.m_hitNormalWorld[i]);
        result.fraction = allConvex.m_hitFractions[i];
    }
    return hits;
}
