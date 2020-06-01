
#pragma once

#include "std.h"
#include "physics/CollisionObject.h"

class Trigger : public CollisionObject
{
public:
    Trigger() : CollisionObject(get_id(Trigger)) {}
    
    virtual class btCollisionObject* constructObject(class btCollisionShape* shape,
        class btMotionState* motion) override;
    
    std::vector<std::shared_ptr<CollisionObject>> getOverlaps();
private:
    std::vector<std::weak_ptr<CollisionObject>> overlaps;

    friend class PhysicsSystem;
};
