
#include "physics/Trigger.h"
#include <bullet/BulletCollision/CollisionDispatch/btGhostObject.h>
#include <bullet/BulletCollision/CollisionShapes/btCollisionShape.h>

btCollisionObject* Trigger::constructObject(class btCollisionShape* shape, class btMotionState* motion)
{
    btGhostObject* obj = new btGhostObject();
    obj->setCollisionFlags(obj->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
    obj->setCollisionShape(shape);
    return obj;
}

vector<shared_ptr<CollisionObject>> Trigger::getOverlaps()
{
    vector<shared_ptr<CollisionObject>> result;
    for(weak_ptr<CollisionObject>& overlap : overlaps) {
        shared_ptr<CollisionObject> overlapPtr = overlap.lock();
        if(overlapPtr) {
            result.push_back(overlapPtr);
        }
    }
    return result;
}
