
#include "physics/ConvexCollider.h"

void ConvexCollider::setConvexHull(ResourceRef<ConvexHull> newHull)
{
    convexHull = newHull;
    convexHull.resolve(Deferred);
    shapeUpdated = true;
}

btCollisionShape* ConvexCollider::constructShape()
{
    std::shared_ptr<ConvexHull> hull = convexHull.resolve(Immediate);
    if(!hull) {
        return nullptr;
    }

    return hull->createHullInstance();
}
