
#pragma once

#include "std.h"
#include "physics/Collider.h"

#include "ComponentTypes.h"

class BoxCollider : public Collider
{
public:
    BoxCollider() : Collider(BOX_COLLIDER_ID) {}

    void setExtents(const glm::vec3& _extents);
    inline glm::vec3 getExtents() const { return extents; }

    virtual btCollisionShape* constructShape() override;
protected:
    glm::vec3 extents = glm::vec3(1,1,1);
};
