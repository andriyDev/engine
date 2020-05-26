
#include "physics/RigidBody.h"
#include <bullet/BulletDynamics/Dynamics/btRigidBody.h>
#include <bullet/BulletCollision/CollisionShapes/btCollisionShape.h>

btCollisionObject* RigidBody::constructObject(btCollisionShape* shape, btMotionState* motion)
{
    btVector3 inertia;
    shape->calculateLocalInertia(mass, inertia);
    btRigidBody* r = new btRigidBody(mass, motion, shape, inertia);
    return r;
}
