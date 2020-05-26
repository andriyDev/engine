
#pragma once

#include "std.h"
#include "components/Transform.h"

class Collider : public Transformable
{
public:
    Collider(uint typeId) : Transformable(typeId) {}

    virtual class btCollisionShape* constructShape() = 0;
protected:
    bool shapeUpdated = false;

    friend class PhysicsSystem;
};
