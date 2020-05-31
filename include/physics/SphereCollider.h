
#pragma once

#include "std.h"
#include "physics/Collider.h"

class SphereCollider : public Collider
{
public:
    SphereCollider() : Collider(get_id(SphereCollider)) {}

    void setRadius(float _radius);
    inline float getRadius() const { return radius; }

    virtual btCollisionShape* constructShape() override;
protected:
    float radius;
};
