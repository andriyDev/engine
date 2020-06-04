
#pragma once

#include "std.h"
#include "core/System.h"
#include <glm/glm.hpp>

class Entity;
class Collider;
class CollisionObject;
class ConvexHull;

struct RaycastHit
{
    bool valid = false;
    vec3 point;
    float fraction;
    vec3 normal;
    weak_ptr<CollisionObject> obj;

    inline shared_ptr<CollisionObject> getObj() const { return obj.lock(); }

    operator bool() const {
        return valid;
    }
};

class PhysicsSystem : public System
{
public:
    virtual ~PhysicsSystem();

    virtual void init() override;
    virtual void gameplayTick(float delta) override;

    void setGravity(const vec3& _gravity);
    vec3 getGravity() const { return gravity; }

    RaycastHit rayCast(const vec3& source, const vec3& direction, float range,
        shared_ptr<Entity> ignoredEntity = nullptr, bool hitTriggers = false) const;

    RaycastHit rayCast(const vec3& source, const vec3& direction, float range,
        const uset<shared_ptr<Entity>>& ignoredEntities,
        const uset<shared_ptr<CollisionObject>>& ignoredBodies,
        bool hitTriggers = false) const;
        
    vector<RaycastHit> rayCastAll(const vec3& source, const vec3& direction, float range,
        shared_ptr<Entity> ignoredEntity = nullptr, bool hitTriggers = false) const;

    vector<RaycastHit> rayCastAll(const vec3& source, const vec3& direction, float range,
        const uset<shared_ptr<Entity>>& ignoredEntities,
        const uset<shared_ptr<CollisionObject>>& ignoredBodies,
        bool hitTriggers = false) const;

    RaycastHit boxCast(const vec3& extents,
        const vec3& sourcePosition, const quat& sourceRotation,
        const vec3& targetPosition, const quat& targetRotation,
        shared_ptr<Entity> ignoredEntity = nullptr, bool hitTriggers = false) const;

    RaycastHit boxCast(const vec3& extents,
        const vec3& sourcePosition, const quat& sourceRotation,
        const vec3& targetPosition, const quat& targetRotation,
        const uset<shared_ptr<Entity>>& ignoredEntities,
        const uset<shared_ptr<CollisionObject>>& ignoredBodies, bool hitTriggers = false) const;
        
    vector<RaycastHit> boxCastAll(const vec3& extents,
        const vec3& sourcePosition, const quat& sourceRotation,
        const vec3& targetPosition, const quat& targetRotation,
        shared_ptr<Entity> ignoredEntity = nullptr, bool hitTriggers = false) const;

    vector<RaycastHit> boxCastAll(const vec3& extents,
        const vec3& sourcePosition, const quat& sourceRotation,
        const vec3& targetPosition, const quat& targetRotation,
        const uset<shared_ptr<Entity>>& ignoredEntities,
        const uset<shared_ptr<CollisionObject>>& ignoredBodies,
        bool hitTriggers = false) const;

    RaycastHit sphereCast(float radius,
        const vec3& sourcePosition, const quat& sourceRotation,
        const vec3& targetPosition, const quat& targetRotation,
        shared_ptr<Entity> ignoredEntity = nullptr, bool hitTriggers = false) const;

    RaycastHit sphereCast(float radius,
        const vec3& sourcePosition, const quat& sourceRotation,
        const vec3& targetPosition, const quat& targetRotation,
        const uset<shared_ptr<Entity>>& ignoredEntities,
        const uset<shared_ptr<CollisionObject>>& ignoredBodies, bool hitTriggers = false) const;
        
    vector<RaycastHit> sphereCastAll(float radius,
        const vec3& sourcePosition, const quat& sourceRotation,
        const vec3& targetPosition, const quat& targetRotation,
        shared_ptr<Entity> ignoredEntity = nullptr, bool hitTriggers = false) const;

    vector<RaycastHit> sphereCastAll(float radius,
        const vec3& sourcePosition, const quat& sourceRotation,
        const vec3& targetPosition, const quat& targetRotation,
        const uset<shared_ptr<Entity>>& ignoredEntities,
        const uset<shared_ptr<CollisionObject>>& ignoredBodies,
        bool hitTriggers = false) const;

    RaycastHit convexCast(shared_ptr<ConvexHull> convex,
        const vec3& sourcePosition, const quat& sourceRotation,
        const vec3& targetPosition, const quat& targetRotation,
        shared_ptr<Entity> ignoredEntity = nullptr, bool hitTriggers = false) const;

    RaycastHit convexCast(shared_ptr<ConvexHull> convex,
        const vec3& sourcePosition, const quat& sourceRotation,
        const vec3& targetPosition, const quat& targetRotation,
        const uset<shared_ptr<Entity>>& ignoredEntities,
        const uset<shared_ptr<CollisionObject>>& ignoredBodies, bool hitTriggers = false) const;
        
    vector<RaycastHit> convexCastAll(shared_ptr<ConvexHull> convex,
        const vec3& sourcePosition, const quat& sourceRotation,
        const vec3& targetPosition, const quat& targetRotation,
        shared_ptr<Entity> ignoredEntity = nullptr, bool hitTriggers = false) const;

    vector<RaycastHit> convexCastAll(shared_ptr<ConvexHull> convex,
        const vec3& sourcePosition, const quat& sourceRotation,
        const vec3& targetPosition, const quat& targetRotation,
        const uset<shared_ptr<Entity>>& ignoredEntities,
        const uset<shared_ptr<CollisionObject>>& ignoredBodies,
        bool hitTriggers = false) const;
protected:

    RaycastHit shapeCast(const class btConvexShape* shape,
        const vec3& sourcePosition, const quat& sourceRotation,
        const vec3& targetPosition, const quat& targetRotation,
        shared_ptr<Entity> ignoredEntity = nullptr, bool hitTriggers = false) const;

    RaycastHit shapeCast(const class btConvexShape* shape,
        const vec3& sourcePosition, const quat& sourceRotation,
        const vec3& targetPosition, const quat& targetRotation,
        const uset<shared_ptr<Entity>>& ignoredEntities,
        const uset<shared_ptr<CollisionObject>>& ignoredBodies, bool hitTriggers = false) const;
        
    vector<RaycastHit> shapeCastAll(const class btConvexShape* shape,
        const vec3& sourcePosition, const quat& sourceRotation,
        const vec3& targetPosition, const quat& targetRotation,
        shared_ptr<Entity> ignoredEntity = nullptr, bool hitTriggers = false) const;

    vector<RaycastHit> shapeCastAll(const class btConvexShape* shape,
        const vec3& sourcePosition, const quat& sourceRotation,
        const vec3& targetPosition, const quat& targetRotation,
        const uset<shared_ptr<Entity>>& ignoredEntities,
        const uset<shared_ptr<CollisionObject>>& ignoredBodies,
        bool hitTriggers = false) const;

    vec3 gravity = vec3(0,-9.81f,0);

    class btCollisionConfiguration* configuration = nullptr;
    class btCollisionDispatcher* dispatcher = nullptr;
    class btBroadphaseInterface* broadphase = nullptr;
    class btConstraintSolver* solver = nullptr;
    class btGhostPairCallback* triggerCallback = nullptr;
    class btDiscreteDynamicsWorld* physicsWorld = nullptr;

    struct CollisionObjectData
    {
        enum Type
        {
            RigidBody,
            Generic
        };

        class btCollisionObject* collisionObject;
        class btCompoundShape* compoundShape;
        class btMotionState* motionState;
        Type type;
        uint updateId;
        std::map<std::weak_ptr<Collider>,
            std::pair<class btCollisionShape*, uint>,
            std::owner_less<>> shapeMap;
    };

    friend class TransformMotionState;

    std::map<std::weak_ptr<CollisionObject>, CollisionObjectData, std::owner_less<>> collisionObjects;
    std::map<btCollisionObject*, std::weak_ptr<CollisionObject>> reverseObjects;

    // Deletes everything associated with the specified body (does not remove the body from the collisionObjects map).
    void cleanUpCollisionObject(CollisionObjectData& body);
    // Constructs a new collisionObject from its component.
    void setUpCollisionObject(shared_ptr<CollisionObject>& bodyComponent);
    // Updates the existing collision object to match the collider components.
    void updateCollidersOfObject(shared_ptr<CollisionObject>& bodyComponent, CollisionObjectData& bodyData);
    // Updates the existing collision object to match the components (applying forces).
    void updateStateOfObject(shared_ptr<CollisionObject>& bodyComponent, CollisionObjectData& bodyData);

    void addBody(CollisionObjectData& body);
    void removeBody(CollisionObjectData& body);
};
