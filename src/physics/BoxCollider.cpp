
#include "physics/BoxCollider.h"

#include <bullet/BulletCollision/CollisionShapes/btBoxShape.h>

btCollisionShape* BoxCollider::constructShape()
{
    return new btBoxShape(
        btVector3(btScalar(extents.x), btScalar(extents.y), btScalar(extents.z))
    );
}

void BoxCollider::setExtents(const glm::vec3& _extents)
{
    extents = _extents;
    shapeUpdated = true;
}
