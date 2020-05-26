
#pragma once

#include "std.h"
#include "physics/Collider.h"

#include "ComponentTypes.h"

class SphereCollider : public Collider
{
public:
    SphereCollider() : Collider(SPHERE_COLLIDER_ID) {}

    void setRadius(float _radius);
    inline float getRadius() const { return radius; }

    virtual btCollisionShape* constructShape() override;
protected:
    float radius;
};
