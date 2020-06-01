
#pragma once

#include "std.h"
#include "Collider.h"
#include "ConvexHull.h"

class ConvexCollider : public Collider
{
public:
    ConvexCollider() : Collider(get_id(ConvexCollider)) {}

    void setConvexHull(ResourceRef<ConvexHull> newHull);
    
    virtual btCollisionShape* constructShape() override;
protected:
    ResourceRef<ConvexHull> convexHull;
};
