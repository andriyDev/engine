
#include "physics/RigidBody.h"
#include <bullet/BulletDynamics/Dynamics/btRigidBody.h>
#include <bullet/BulletCollision/CollisionShapes/btCollisionShape.h>

#include "physics/BulletUtil.h"

void RigidBody::addForce(const vec3& force)
{
    if(getBody()) {
        btRigidBody* rbbody = static_cast<btRigidBody*>(getBody());
        rbbody->applyCentralForce(convert(force));
    }
}

void RigidBody::addPointForce(const vec3& force, vec3 worldPoint)
{
    if(getBody()) {
        btRigidBody* rbbody = static_cast<btRigidBody*>(getBody());
        shared_ptr<Transform> transform = getTransform();
        if(transform) {
            TransformData td = transform->getGlobalTransform();
            td.scale = vec3(1,1,1);
            worldPoint = td.inverse().transformPoint(worldPoint);
        }
        rbbody->applyForce(convert(force), convert(worldPoint));
    }
}

void RigidBody::addImpulse(const vec3& impulse)
{
    if(getBody()) {
        btRigidBody* rbbody = static_cast<btRigidBody*>(getBody());
        rbbody->applyCentralImpulse(convert(impulse));
    }
}

void RigidBody::addPointImpulse(const vec3& impulse, vec3 worldPoint)
{
    if(getBody()) {
        btRigidBody* rbbody = static_cast<btRigidBody*>(getBody());
        shared_ptr<Transform> transform = getTransform();
        if(transform) {
            TransformData td = transform->getGlobalTransform();
            td.scale = vec3(1,1,1);
            worldPoint = td.inverse().transformPoint(worldPoint);
        }
        rbbody->applyImpulse(convert(impulse), convert(worldPoint));
    }
}

void RigidBody::addTorque(const vec3& torque)
{
    if(getBody()) {
        btRigidBody* rbbody = static_cast<btRigidBody*>(getBody());
        rbbody->applyTorque(convert(torque));
    }
}

void RigidBody::addTorqueImpulse(const vec3& torque)
{
    if(getBody()) {
        btRigidBody* rbbody = static_cast<btRigidBody*>(getBody());
        rbbody->applyTorqueImpulse(convert(torque));
    }
}

btCollisionObject* RigidBody::constructObject(btCollisionShape* shape, btMotionState* motion)
{
    btVector3 inertia;
    shape->calculateLocalInertia(mass, inertia);
    btRigidBody* r = new btRigidBody(mass, motion, shape, inertia);
    return r;
}
