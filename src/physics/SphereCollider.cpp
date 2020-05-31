
#include "physics/SphereCollider.h"

#include <bullet/BulletCollision/CollisionShapes/btSphereShape.h>

btCollisionShape* SphereCollider::constructShape()
{
    return new btSphereShape(btScalar(radius));
}

void SphereCollider::setRadius(float _radius)
{
    radius = _radius;
    shapeUpdated = true;
}
