
#pragma once

#include "std.h"
#include "physics/CollisionObject.h"

class RigidBody : public CollisionObject
{
public:
    RigidBody() : CollisionObject(RIGIDBODY_ID) {}

    float mass = 1;

    void addForce(const glm::vec3& force);
    void addPointForce(const glm::vec3& force, glm::vec3 worldPoint);
    void addImpulse(const glm::vec3& impulse);
    void addPointImpulse(const glm::vec3& impulse, glm::vec3 worldPoint);
    void addTorque(const glm::vec3& torque);
    void addTorqueImpulse(const glm::vec3& torque);

    virtual class btCollisionObject* constructObject(class btCollisionShape* shape,
        class btMotionState* motion) override;
};
