
#include "physics/PhysicsSystem.h"
#include <bullet/BulletCollision/CollisionDispatch/btGhostObject.h>
#include <bullet/BulletCollision/NarrowPhaseCollision/btRaycastCallback.h>
#include <bullet/btBulletDynamicsCommon.h>

#include "physics/CollisionObject.h"
#include "physics/ConvexHull.h"
#include "physics/BulletUtil.h"

struct FilterRaysCallback : public btCollisionWorld::RayResultCallback
{
public:
    btCollisionWorld::RayResultCallback* wrappedCallback;
    const map<btCollisionObject*, weak_ptr<CollisionObject>>* reverseObjects;
    bool hitTriggers;
    const uset<shared_ptr<CollisionObject>>* ignoredBodies;
    const uset<shared_ptr<Entity>>* ignoreEntities;

    FilterRaysCallback(btCollisionWorld::RayResultCallback* _wrappedCallback,
        const map<btCollisionObject*, weak_ptr<CollisionObject>>* _reverseObjects)
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

RaycastHit PhysicsSystem::rayCast(const vec3& source, const vec3& direction, float range,
    shared_ptr<Entity> ignoredEntity, bool hitTriggers) const
{
    uset<shared_ptr<Entity>> ignoredEntities;
    ignoredEntities.insert(ignoredEntity);
    return rayCast(source, direction, range, ignoredEntities,
        uset<shared_ptr<CollisionObject>>(), hitTriggers);
}

RaycastHit PhysicsSystem::rayCast(const vec3& source, const vec3& direction, float range,
    const uset<shared_ptr<Entity>>& ignoredEntities,
    const uset<shared_ptr<CollisionObject>>& ignoredBodies,
    bool hitTriggers) const
{
    RaycastHit result;
    result.valid = false;
    result.point = source + direction * range;
    result.normal = vec3(0,0,0);
    result.obj = shared_ptr<CollisionObject>();
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
        
vector<RaycastHit> PhysicsSystem::rayCastAll(const vec3& source, const vec3& direction, 
    float range, shared_ptr<Entity> ignoredEntity, bool hitTriggers) const
{
    uset<shared_ptr<Entity>> ignoredEntities;
    ignoredEntities.insert(ignoredEntity);
    return rayCastAll(source, direction, range, ignoredEntities,
        uset<shared_ptr<CollisionObject>>(), hitTriggers);
}

vector<RaycastHit> PhysicsSystem::rayCastAll(const vec3& source, const vec3& direction,
    float range, const uset<shared_ptr<Entity>>& ignoredEntities,
    const uset<shared_ptr<CollisionObject>>& ignoredBodies,
    bool hitTriggers) const
{
    vector<RaycastHit> hits;
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
    const map<btCollisionObject*, weak_ptr<CollisionObject>>* reverseObjects;
    bool hitTriggers;
    const uset<shared_ptr<CollisionObject>>* ignoredBodies;
    const uset<shared_ptr<Entity>>* ignoreEntities;

    FilterConvexCallback(btCollisionWorld::ConvexResultCallback* _wrappedCallback,
        const map<btCollisionObject*, weak_ptr<CollisionObject>>* _reverseObjects)
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
    const vec3& sourcePosition, const quat& sourceRotation,
    const vec3& targetPosition, const quat& targetRotation,
    shared_ptr<Entity> ignoredEntity, bool hitTriggers) const
{
    uset<shared_ptr<Entity>> ignoredEntities;
    ignoredEntities.insert(ignoredEntity);
    return shapeCast(shape, sourcePosition, sourceRotation, targetPosition, targetRotation,
        ignoredEntities, uset<shared_ptr<CollisionObject>>(), hitTriggers);
}

RaycastHit PhysicsSystem::shapeCast(const class btConvexShape* shape,
    const vec3& sourcePosition, const quat& sourceRotation,
    const vec3& targetPosition, const quat& targetRotation,
    const uset<shared_ptr<Entity>>& ignoredEntities,
    const uset<shared_ptr<CollisionObject>>& ignoredBodies, bool hitTriggers) const
{
    RaycastHit result;
    result.valid = false;
    result.point = targetPosition;
    result.normal = vec3(0,0,0);
    result.obj = shared_ptr<CollisionObject>();
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
    
vector<RaycastHit> PhysicsSystem::shapeCastAll(const class btConvexShape* shape,
    const vec3& sourcePosition, const quat& sourceRotation,
    const vec3& targetPosition, const quat& targetRotation,
    shared_ptr<Entity> ignoredEntity, bool hitTriggers) const
{
    uset<shared_ptr<Entity>> ignoredEntities;
    ignoredEntities.insert(ignoredEntity);
    return shapeCastAll(shape, sourcePosition, sourceRotation, targetPosition, targetRotation,
        ignoredEntities, uset<shared_ptr<CollisionObject>>(), hitTriggers);
}

vector<RaycastHit> PhysicsSystem::shapeCastAll(const class btConvexShape* shape,
    const vec3& sourcePosition, const quat& sourceRotation,
    const vec3& targetPosition, const quat& targetRotation,
    const uset<shared_ptr<Entity>>& ignoredEntities,
    const uset<shared_ptr<CollisionObject>>& ignoredBodies,
    bool hitTriggers) const
{
    vector<RaycastHit> hits;
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

RaycastHit PhysicsSystem::boxCast(const vec3& extents,
    const vec3& sourcePosition, const quat& sourceRotation,
    const vec3& targetPosition, const quat& targetRotation,
    shared_ptr<Entity> ignoredEntity, bool hitTriggers) const
{
    btBoxShape shape(convert(extents));
    return shapeCast(&shape, sourcePosition, sourceRotation, targetPosition, targetRotation,
        ignoredEntity, hitTriggers);
}

RaycastHit PhysicsSystem::boxCast(const vec3& extents,
    const vec3& sourcePosition, const quat& sourceRotation,
    const vec3& targetPosition, const quat& targetRotation,
    const uset<shared_ptr<Entity>>& ignoredEntities,
    const uset<shared_ptr<CollisionObject>>& ignoredBodies, bool hitTriggers) const
{
    btBoxShape shape(convert(extents));
    return shapeCast(&shape, sourcePosition, sourceRotation, targetPosition, targetRotation,
        ignoredEntities, ignoredBodies, hitTriggers);
}
    
vector<RaycastHit> PhysicsSystem::boxCastAll(const vec3& extents,
    const vec3& sourcePosition, const quat& sourceRotation,
    const vec3& targetPosition, const quat& targetRotation,
    shared_ptr<Entity> ignoredEntity, bool hitTriggers) const
{
    btBoxShape shape(convert(extents));
    return shapeCastAll(&shape, sourcePosition, sourceRotation, targetPosition, targetRotation,
        ignoredEntity, hitTriggers);
}

vector<RaycastHit> PhysicsSystem::boxCastAll(const vec3& extents,
    const vec3& sourcePosition, const quat& sourceRotation,
    const vec3& targetPosition, const quat& targetRotation,
    const uset<shared_ptr<Entity>>& ignoredEntities,
    const uset<shared_ptr<CollisionObject>>& ignoredBodies,
    bool hitTriggers) const
{
    btBoxShape shape(convert(extents));
    return shapeCastAll(&shape, sourcePosition, sourceRotation, targetPosition, targetRotation,
        ignoredEntities, ignoredBodies, hitTriggers);
}

RaycastHit PhysicsSystem::sphereCast(float radius,
    const vec3& sourcePosition, const quat& sourceRotation,
    const vec3& targetPosition, const quat& targetRotation,
    shared_ptr<Entity> ignoredEntity, bool hitTriggers) const
{
    btSphereShape shape(radius);
    return shapeCast(&shape, sourcePosition, sourceRotation, targetPosition, targetRotation,
        ignoredEntity, hitTriggers);
}

RaycastHit PhysicsSystem::sphereCast(float radius,
    const vec3& sourcePosition, const quat& sourceRotation,
    const vec3& targetPosition, const quat& targetRotation,
    const uset<shared_ptr<Entity>>& ignoredEntities,
    const uset<shared_ptr<CollisionObject>>& ignoredBodies, bool hitTriggers) const
{
    btSphereShape shape(radius);
    return shapeCast(&shape, sourcePosition, sourceRotation, targetPosition, targetRotation,
        ignoredEntities, ignoredBodies, hitTriggers);
}
    
vector<RaycastHit> PhysicsSystem::sphereCastAll(float radius,
    const vec3& sourcePosition, const quat& sourceRotation,
    const vec3& targetPosition, const quat& targetRotation,
    shared_ptr<Entity> ignoredEntity, bool hitTriggers) const
{
    btSphereShape shape(radius);
    return shapeCastAll(&shape, sourcePosition, sourceRotation, targetPosition, targetRotation,
        ignoredEntity, hitTriggers);
}

vector<RaycastHit> PhysicsSystem::sphereCastAll(float radius,
    const vec3& sourcePosition, const quat& sourceRotation,
    const vec3& targetPosition, const quat& targetRotation,
    const uset<shared_ptr<Entity>>& ignoredEntities,
    const uset<shared_ptr<CollisionObject>>& ignoredBodies,
    bool hitTriggers) const
{
    btSphereShape shape(radius);
    return shapeCastAll(&shape, sourcePosition, sourceRotation, targetPosition, targetRotation,
        ignoredEntities, ignoredBodies, hitTriggers);
}

RaycastHit PhysicsSystem::convexCast(shared_ptr<ConvexHull> convex,
    const vec3& sourcePosition, const quat& sourceRotation,
    const vec3& targetPosition, const quat& targetRotation,
    shared_ptr<Entity> ignoredEntity, bool hitTriggers) const
{
    assert(convex->shape);
    return shapeCast(convex->shape, sourcePosition, sourceRotation, targetPosition, targetRotation,
        ignoredEntity, hitTriggers);
}

RaycastHit PhysicsSystem::convexCast(shared_ptr<ConvexHull> convex,
    const vec3& sourcePosition, const quat& sourceRotation,
    const vec3& targetPosition, const quat& targetRotation,
    const uset<shared_ptr<Entity>>& ignoredEntities,
    const uset<shared_ptr<CollisionObject>>& ignoredBodies, bool hitTriggers) const
{
    assert(convex->shape);
    return shapeCast(convex->shape, sourcePosition, sourceRotation, targetPosition, targetRotation,
        ignoredEntities, ignoredBodies, hitTriggers);
}
    
vector<RaycastHit> PhysicsSystem::convexCastAll(shared_ptr<ConvexHull> convex,
    const vec3& sourcePosition, const quat& sourceRotation,
    const vec3& targetPosition, const quat& targetRotation,
    shared_ptr<Entity> ignoredEntity, bool hitTriggers) const
{
    assert(convex->shape);
    return shapeCastAll(convex->shape, sourcePosition, sourceRotation, targetPosition, targetRotation,
        ignoredEntity, hitTriggers);
}

vector<RaycastHit> PhysicsSystem::convexCastAll(shared_ptr<ConvexHull> convex,
    const vec3& sourcePosition, const quat& sourceRotation,
    const vec3& targetPosition, const quat& targetRotation,
    const uset<shared_ptr<Entity>>& ignoredEntities,
    const uset<shared_ptr<CollisionObject>>& ignoredBodies,
    bool hitTriggers) const
{
    assert(convex->shape);
    return shapeCastAll(convex->shape, sourcePosition, sourceRotation, targetPosition, targetRotation,
        ignoredEntities, ignoredBodies, hitTriggers);
}
