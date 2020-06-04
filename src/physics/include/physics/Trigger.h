
#pragma once

#include "std.h"
#include "physics/CollisionObject.h"

class Trigger : public CollisionObject
{
public:
    Trigger() : CollisionObject(get_id(Trigger)) {}
    
    virtual class btCollisionObject* constructObject(class btCollisionShape* shape,
        class btMotionState* motion) override;
    
    vector<shared_ptr<CollisionObject>> getOverlaps();
private:
    vector<weak_ptr<CollisionObject>> overlaps;

    friend class PhysicsSystem;
};
