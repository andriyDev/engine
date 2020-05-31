
#pragma once

#include "std.h"
#include "core/System.h"
#include <glm/glm.hpp>

class Entity;
class Collider;
class CollisionObject;

struct RaycastHit
{
    bool valid = false;
    glm::vec3 point;
    float fraction;
    glm::vec3 normal;
    std::weak_ptr<CollisionObject> obj;

    inline std::shared_ptr<CollisionObject> getObj() const { return obj.lock(); }

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

    void setGravity(const glm::vec3& _gravity);
    glm::vec3 getGravity() const { return gravity; }

    RaycastHit rayCast(const glm::vec3& source, const glm::vec3& direction, float range,
        std::shared_ptr<Entity> ignoredEntity = nullptr, bool hitTriggers = false) const;

    RaycastHit rayCast(const glm::vec3& source, const glm::vec3& direction, float range,
        const std::set<std::shared_ptr<Entity>>& ignoredEntities,
        const std::set<std::shared_ptr<CollisionObject>>& ignoredBodies,
        bool hitTriggers = false) const;
        
    std::vector<RaycastHit> rayCastAll(const glm::vec3& source, const glm::vec3& direction, float range,
        std::shared_ptr<Entity> ignoredEntity = nullptr, bool hitTriggers = false) const;

    std::vector<RaycastHit> rayCastAll(const glm::vec3& source, const glm::vec3& direction, float range,
        const std::set<std::shared_ptr<Entity>>& ignoredEntities,
        const std::set<std::shared_ptr<CollisionObject>>& ignoredBodies,
        bool hitTriggers = false) const;

    RaycastHit shapeCast(const class btConvexShape* shape,
        const glm::vec3& sourcePosition, const glm::quat& sourceRotation,
        const glm::vec3& targetPosition, const glm::quat& targetRotation,
        std::shared_ptr<Entity> ignoredEntity = nullptr, bool hitTriggers = false) const;

    RaycastHit shapeCast(const class btConvexShape* shape,
        const glm::vec3& sourcePosition, const glm::quat& sourceRotation,
        const glm::vec3& targetPosition, const glm::quat& targetRotation,
        const std::set<std::shared_ptr<Entity>>& ignoredEntities,
        const std::set<std::shared_ptr<CollisionObject>>& ignoredBodies, bool hitTriggers = false) const;
        
    std::vector<RaycastHit> shapeCastAll(const class btConvexShape* shape,
        const glm::vec3& sourcePosition, const glm::quat& sourceRotation,
        const glm::vec3& targetPosition, const glm::quat& targetRotation,
        std::shared_ptr<Entity> ignoredEntity = nullptr, bool hitTriggers = false) const;

    std::vector<RaycastHit> shapeCastAll(const class btConvexShape* shape,
        const glm::vec3& sourcePosition, const glm::quat& sourceRotation,
        const glm::vec3& targetPosition, const glm::quat& targetRotation,
        const std::set<std::shared_ptr<Entity>>& ignoredEntities,
        const std::set<std::shared_ptr<CollisionObject>>& ignoredBodies,
        bool hitTriggers = false) const;
protected:
    glm::vec3 gravity = glm::vec3(0,-9.81f,0);

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
    void setUpCollisionObject(std::shared_ptr<CollisionObject>& bodyComponent);
    // Updates the existing collision object to match the collider components.
    void updateCollidersOfObject(std::shared_ptr<CollisionObject>& bodyComponent, CollisionObjectData& bodyData);
    // Updates the existing collision object to match the components (applying forces).
    void updateStateOfObject(std::shared_ptr<CollisionObject>& bodyComponent, CollisionObjectData& bodyData);
};
