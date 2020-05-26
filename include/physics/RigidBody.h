
#pragma once

#include "std.h"
#include "physics/CollisionObject.h"

class RigidBody : public CollisionObject
{
public:
    RigidBody() : CollisionObject(RIGIDBODY_ID) {}

    float mass = 1;

    void addForce(const glm::vec3& force);

    virtual class btCollisionObject* constructObject(class btCollisionShape* shape,
        class btMotionState* motion) override;
};
