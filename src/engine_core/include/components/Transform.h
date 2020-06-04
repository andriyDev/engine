
#pragma once

#include "std.h"

#include "core/Component.h"

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
    TransformData inverse() const;

    // Applies translation, rotation and scaling to the point so it is relative to the transform's reference frame.
    vec3 transformPoint(const vec3& point) const;
    // Applies rotation to the direction so it is relative to the transform's reference frame.
    vec3 transformDirection(const vec3& direction) const;
    // Applies rotation and scaling to the direction so it is relative to the transform's reference frame.
    vec3 transformDirectionWithScale(const vec3& direction) const;

    vec3 forward() const { return transformDirection(vec3(0, 0, -1)); }
    vec3 right() const { return transformDirection(vec3(1, 0, 0)); }
    vec3 up() const { return transformDirection(vec3(0, 1, 0)); }

    TransformData lerp(TransformData other, float alpha) const;

    mat4 toMat4() const;

    TransformData& operator*=(const TransformData& rhs);
    friend TransformData& operator*(TransformData lhs, const TransformData& rhs) {
        return lhs *= rhs;
    }
};

string to_string(const TransformData& data);

class Transform : public Component
{
public:
    Transform() : Component(get_id(Transform)) {}
    virtual ~Transform();

    // Gets the relative transform.
    TransformData getRelativeTransform() const {
        return relativeTransform;
    }
    // Sets the relative transform.
    void setRelativeTransform(const TransformData& relativeTransform);
    // Gets the global transform of this component.
    TransformData getGlobalTransform() const;
    // Sets the relative transform so that it matches globally.
    void setGlobalTransform(const TransformData& globalTransform);
    // Gets the transform of this component relative to the provided transform.
    TransformData getTransformRelativeTo(shared_ptr<Transform> relative) const;
    // Sets the transform of this component relative to the provided transform. Only this transform will be moved.
    void setTransformRelativeTo(const TransformData& transform, shared_ptr<Transform> relative);

    /*
    Replaces the current parent with the newParent (can be null to attach to world).
    If keepGlobal = true, then the global transform will not change after parent is set.
    */
    void setParent(shared_ptr<Transform> newParent, bool keepGlobal);

    // Gets the parent of this transform.
    shared_ptr<Transform> getParent() const;

    // Gets the children of this transform.
    vector<shared_ptr<Transform>> getChildren() const;

    inline uint getUpdateId() const { return updateId; }

    uint sumUpdatesRelativeTo(shared_ptr<Transform> relative) const;
    uint sumUpdates() const { return sumUpdatesRelativeTo(nullptr); }
protected:
    void incrementUpdateId();
private:
    TransformData relativeTransform; // The transform data relative to this transform's parent.
    /*
    Stores a number to identify this value of the transform. If the transform is changed, this value is changed.
    */
    uint updateId = 0;
    weak_ptr<Transform> parent; // The parent of this transform.
    vector<weak_ptr<Transform>> children; // The children of this transform.
};

class Transformable : public Component
{
public:
    Transformable(uint typeId) : Component(typeId) {}

    weak_ptr<Transform> transform;

    inline shared_ptr<Transform> getTransform() const {
        return transform.lock();
    }
};

shared_ptr<Transform> mapToTransform(shared_ptr<Transformable> component);
