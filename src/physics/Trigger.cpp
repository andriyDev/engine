
#include "physics/Trigger.h"
#include <bullet/BulletCollision/CollisionDispatch/btGhostObject.h>
#include <bullet/BulletCollision/CollisionShapes/btCollisionShape.h>

btCollisionObject* Trigger::constructObject(class btCollisionShape* shape, class btMotionState* motion)
{
    btGhostObject* obj = new btGhostObject();
    obj->setCollisionShape(shape);
    return obj;
}

std::vector<std::shared_ptr<CollisionObject>> Trigger::getOverlaps()
{
    std::vector<std::shared_ptr<CollisionObject>> result;
    for(std::weak_ptr<CollisionObject>& overlap : overlaps) {
        std::shared_ptr<CollisionObject> overlapPtr = overlap.lock();
        if(overlapPtr) {
            result.push_back(overlapPtr);
        }
    }
    return result;
}
