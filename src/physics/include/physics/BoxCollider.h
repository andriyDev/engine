
#pragma once

#include "std.h"
#include "physics/Collider.h"

class BoxCollider : public Collider
{
public:
    BoxCollider() : Collider(get_id(BoxCollider)) {}

    void setExtents(const glm::vec3& _extents);
    inline glm::vec3 getExtents() const { return extents; }

    virtual btCollisionShape* constructShape() override;
protected:
    glm::vec3 extents = glm::vec3(1,1,1);
};
