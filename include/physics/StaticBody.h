
#pragma once

#include "std.h"
#include "physics/CollisionObject.h"

class StaticBody : public CollisionObject
{
public:
    StaticBody() : CollisionObject(STATICBODY_ID) {}

    float mass = 1;

    virtual class btCollisionObject* constructObject(class btCollisionShape* shape,
        class btMotionState* motion) override;
};
