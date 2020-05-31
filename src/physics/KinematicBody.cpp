
#include "physics/KinematicBody.h"
#include <bullet/BulletDynamics/Dynamics/btRigidBody.h>

btCollisionObject* KinematicBody::constructObject(btCollisionShape* shape, btMotionState* motion)
{
    btRigidBody* r = new btRigidBody(mass, motion, shape);
    r->setCollisionFlags(r->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
    r->setActivationState(DISABLE_DEACTIVATION);
    return r;
}
