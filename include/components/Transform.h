
#pragma once

#include "std.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
using namespace glm;

#include "core/Component.h"

struct TransformData
{
    vec3 position;
    quat rotation;
    vec3 scale;

    TransformData()
        : position(vec3(0,0,0)),
        rotation(quat(1,0,0,0)),
        scale(vec3(1,1,1))
    {}

    // Computes the inverse of this transformation.
    TransformData inverse();

    // Applies translation, rotation and scaling to the point so it is relative to the transform's reference frame.
    vec3 transformPoint(const vec3& point);
    // Applies rotation to the direction so it is relative to the transform's reference frame.
    vec3 transformDirection(const vec3& direction);
    // Applies rotation and scaling to the direction so it is relative to the transform's reference frame.
    vec3 transformDirectionWithScale(const vec3& direction);

    TransformData& operator*=(const TransformData& rhs);
    friend TransformData& operator*(TransformData lhs, const TransformData& rhs) {
        return lhs *= rhs;
    }
};

class Transform : public Component
{
public:
    // The transform data relative to this transform's parent.
    TransformData relativeTransform;

    // Gets the global transform of this component.
    TransformData getGlobalTransform() const;
    // Sets the relative transform so that it matches globally.
    void setGlobalTransform(const TransformData& globalTransform);

    // Replaces the current parent with the newParent (can be null to attach to world).
    void setParent(Transform* newParent);
private:
    Transform* parent; // The parent of this transform.
};
