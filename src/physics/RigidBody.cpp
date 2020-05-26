
#include "physics/RigidBody.h"
#include <bullet/BulletDynamics/Dynamics/btRigidBody.h>
#include <bullet/BulletCollision/CollisionShapes/btCollisionShape.h>

#include "physics/BulletUtil.h"

void RigidBody::addForce(const glm::vec3& force)
{
    if(getBody()) {
        btRigidBody* rbbody = static_cast<btRigidBody*>(getBody());
        rbbody->applyCentralForce(convert(force));
    }
}

btCollisionObject* RigidBody::constructObject(btCollisionShape* shape, btMotionState* motion)
{
    btVector3 inertia;
    shape->calculateLocalInertia(mass, inertia);
    btRigidBody* r = new btRigidBody(mass, motion, shape, inertia);
    r->setActivationState(DISABLE_DEACTIVATION);
    return r;
}
