
#pragma once

#include "std.h"
#include "physics/Collider.h"

class BoxCollider : public Collider
{
public:
    BoxCollider() : Collider(get_id(BoxCollider)) {}

    void setExtents(const vec3& _extents);
    inline vec3 getExtents() const { return extents; }

    virtual btCollisionShape* constructShape() override;
protected:
    vec3 extents = vec3(1,1,1);
};
