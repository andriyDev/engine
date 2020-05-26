
#pragma once

#include "std.h"

#include "core/System.h"
#include "RigidBody.h"

class PhysicsSystem : public System
{
public:
    virtual ~PhysicsSystem();

    virtual void init() override;
    virtual void gameplayTick(float delta) override;

    void setGravity(const glm::vec3& _gravity);
    glm::vec3 getGravity() const { return gravity; }
protected:
    glm::vec3 gravity = glm::vec3(0,-9.81f,0);

    class btCollisionConfiguration* configuration = nullptr;
    class btCollisionDispatcher* dispatcher = nullptr;
    class btBroadphaseInterface* broadphase = nullptr;
    class btConstraintSolver* solver = nullptr;
    class btDiscreteDynamicsWorld* physicsWorld = nullptr;

    struct CollisionObjectData
    {
        class btCollisionObject* collisionObject;
        class btCompoundShape* compoundShape;
        class btMotionState* motionState;
        std::map<std::weak_ptr<Collider>, btCollisionShape*, std::owner_less<>> shapeMap;
    };

    std::map<std::weak_ptr<CollisionObject>, CollisionObjectData, std::owner_less<>> collisionObjects;

    // Deletes everything associated with the specified body (does not remove the body from the collisionObjects map).
    void cleanUpCollisionObject(CollisionObjectData& body);
    // Constructs a new collisionObject from its component.
    void setUpCollisionObject(std::shared_ptr<CollisionObject>& bodyComponent);
    // Updates the existing collision object to match the collider components.
    void updateCollidersOfObject(std::shared_ptr<CollisionObject>& bodyComponent, CollisionObjectData& bodyData);
    // Updates the existing collision object to match the components (applying forces).
    void updateStateOfObject(std::shared_ptr<CollisionObject>& bodyComponent, CollisionObjectData& bodyData);
};
