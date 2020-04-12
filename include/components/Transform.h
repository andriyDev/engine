
#pragma once

#include "std.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
using namespace glm;

#include "core/Component.h"

#include "ComponentTypes.h"

struct TransformData
{
    vec3 translation;
    quat rotation;
    vec3 scale;

    TransformData(
        vec3 _translation = vec3(0,0,0),
        quat _rotation = quat(1,0,0,0),
        vec3 _scale = vec3(1,1,1))
        : translation(_translation),
        rotation(_rotation),
        scale(_scale)
    {}

    // Computes the inverse of this transformation.
    TransformData inverse();

    // Applies translation, rotation and scaling to the point so it is relative to the transform's reference frame.
    vec3 transformPoint(const vec3& point);
    // Applies rotation to the direction so it is relative to the transform's reference frame.
    vec3 transformDirection(const vec3& direction);
    // Applies rotation and scaling to the direction so it is relative to the transform's reference frame.
    vec3 transformDirectionWithScale(const vec3& direction);

    TransformData lerp(TransformData other, float alpha) const;

    mat4 toMat4();

    TransformData& operator*=(const TransformData& rhs);
    friend TransformData& operator*(TransformData lhs, const TransformData& rhs) {
        return lhs *= rhs;
    }
};

string to_string(const TransformData& data);

class Transform : public Component
{
public:
    Transform() : Component(TRANSFORM_ID) {}

    // Gets the relative transform.
    TransformData getRelativeTransform() const {
        return relativeTransform;
    }
    // Gets the relative transform 
    TransformData getRelativeTransform(float interpolation) const {
        return previousTransform[0].lerp(previousTransform[1], interpolation);
    }
    // Sets the relative transform. teleport should be set to true if this set is due to a "sharp" move.
    void setRelativeTransform(const TransformData& relativeTransform, bool teleport=false);
    // Gets the global transform of this component.
    TransformData getGlobalTransform() const;
    // Gets the global transform of this component.
    TransformData getGlobalTransform(float interpolation) const;
    // Sets the relative transform so that it matches globally.
    void setGlobalTransform(const TransformData& globalTransform, bool teleport=false);

    /*
    Replaces the current parent with the newParent (can be null to attach to world).
    If keepGlobal = true, then the global transform will not change after parent is set.
    */
    void setParent(Transform* newParent, bool keepGlobal);

    // Gets the parent of this transform.
    Transform* getParent() const;

    static Transform* getComponentTransform(Component const* comp);

private:
    TransformData relativeTransform; // The transform data relative to this transform's parent.
    TransformData previousTransform[2]; // The transform data from the last 2 frames.
    Transform* parent = nullptr; // The parent of this transform.

    friend class RenderSystem;
};
