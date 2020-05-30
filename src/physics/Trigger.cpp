
#include "physics/Trigger.h"
#include <bullet/BulletCollision/CollisionDispatch/btGhostObject.h>
#include <bullet/BulletCollision/CollisionShapes/btCollisionShape.h>

btCollisionObject* Trigger::constructObject(class btCollisionShape* shape, class btMotionState* motion)
{
    btGhostObject* obj = new btGhostObject();
    obj->setCollisionShape(shape);
    return obj;
}
