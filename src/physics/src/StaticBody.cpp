
#include "physics/StaticBody.h"
#include <bullet/BulletDynamics/Dynamics/btRigidBody.h>

btCollisionObject* StaticBody::constructObject(btCollisionShape* shape, btMotionState* motion)
{
    return new btRigidBody(0, motion, shape);
}
