
#pragma once

#include "std.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "core/Component.h"

struct TransformData
{
    glm::vec3 translation;
    glm::quat rotation;
    glm::vec3 scale;

    TransformData(
        glm::vec3 _translation = glm::vec3(0,0,0),
        glm::quat _rotation = glm::quat(1,0,0,0),
        glm::vec3 _scale = glm::vec3(1,1,1))
        : translation(_translation),
        rotation(_rotation),
        scale(_scale)
    {}

    // Computes the inverse of this transformation.
    TransformData inverse();

    // Applies translation, rotation and scaling to the point so it is relative to the transform's reference frame.
    glm::vec3 transformPoint(const glm::vec3& point);
    // Applies rotation to the direction so it is relative to the transform's reference frame.
    glm::vec3 transformDirection(const glm::vec3& direction);
    // Applies rotation and scaling to the direction so it is relative to the transform's reference frame.
    glm::vec3 transformDirectionWithScale(const glm::vec3& direction);

    TransformData lerp(TransformData other, float alpha) const;

    glm::mat4 toMat4();

    TransformData& operator*=(const TransformData& rhs);
    friend TransformData& operator*(TransformData lhs, const TransformData& rhs) {
        return lhs *= rhs;
    }
};

std::string to_string(const TransformData& data);

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
    TransformData getTransformRelativeTo(std::shared_ptr<Transform> relative) const;
    // Sets the transform of this component relative to the provided transform. Only this transform will be moved.
    void setTransformRelativeTo(const TransformData& transform, std::shared_ptr<Transform> relative);

    /*
    Replaces the current parent with the newParent (can be null to attach to world).
    If keepGlobal = true, then the global transform will not change after parent is set.
    */
    void setParent(std::shared_ptr<Transform> newParent, bool keepGlobal);

    // Gets the parent of this transform.
    std::shared_ptr<Transform> getParent() const;

    // Gets the children of this transform.
    std::vector<std::shared_ptr<Transform>> getChildren() const;

    inline uint getUpdateId() const { return updateId; }

    uint sumUpdatesRelativeTo(std::shared_ptr<Transform> relative) const;
    uint sumUpdates() const { return sumUpdatesRelativeTo(nullptr); }
protected:
    void incrementUpdateId();
private:
    TransformData relativeTransform; // The transform data relative to this transform's parent.
    /*
    Stores a number to identify this value of the transform. If the transform is changed, this value is changed.
    */
    uint updateId = 0;
    std::weak_ptr<Transform> parent; // The parent of this transform.
    std::vector<std::weak_ptr<Transform>> children; // The children of this transform.
};

class Transformable : public Component
{
public:
    Transformable(uint typeId) : Component(typeId) {}

    std::weak_ptr<Transform> transform;

    inline std::shared_ptr<Transform> getTransform() const {
        return transform.lock();
    }
};

std::shared_ptr<Transform> mapToTransform(std::shared_ptr<Transformable> component);
