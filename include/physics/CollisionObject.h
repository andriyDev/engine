
#pragma once

#include "std.h"
#include "components/Transform.h"
#include "Collider.h"

class CollisionObject : public Transformable
{
public:
    CollisionObject(uint typeId) : Transformable(typeId) {}

    virtual class btCollisionObject* constructObject(class btCollisionShape* shape, class btMotionState* motion) = 0;

    std::vector<std::weak_ptr<Collider>> colliders;

    std::vector<std::shared_ptr<Collider>> getColliders();
};
