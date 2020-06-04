
#pragma once

#include "std.h"
#include "physics/CollisionObject.h"

class RigidBody : public CollisionObject
{
public:
    RigidBody() : CollisionObject(get_id(RigidBody)) {}

    float mass = 1;

    void addForce(const vec3& force);
    void addPointForce(const vec3& force, vec3 worldPoint);
    void addImpulse(const vec3& impulse);
    void addPointImpulse(const vec3& impulse, vec3 worldPoint);
    void addTorque(const vec3& torque);
    void addTorqueImpulse(const vec3& torque);

    virtual class btCollisionObject* constructObject(class btCollisionShape* shape,
        class btMotionState* motion) override;
};
