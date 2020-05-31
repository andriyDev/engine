
#pragma once

#include "std.h"
#include "physics/CollisionObject.h"

class StaticBody : public CollisionObject
{
public:
    StaticBody() : CollisionObject(get_id(StaticBody)) {}

    float mass = 1;

    virtual class btCollisionObject* constructObject(class btCollisionShape* shape,
        class btMotionState* motion) override;
};
