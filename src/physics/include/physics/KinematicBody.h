
#pragma once

#include "std.h"
#include "physics/CollisionObject.h"

class KinematicBody : public CollisionObject
{
public:
    KinematicBody() : CollisionObject(get_id(KinematicBody)) {}

    float mass = 1;

    virtual class btCollisionObject* constructObject(class btCollisionShape* shape,
        class btMotionState* motion) override;
};
