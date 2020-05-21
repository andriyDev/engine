
#include "components/Transform.h"
#include "core/Entity.h"
#include "core/Universe.h"

#include <sstream>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/transform.hpp>

void Transform::setParent(std::shared_ptr<Transform> newParent, bool keepGlobal)
{
    std::shared_ptr<Transform> parentPtr = parent.lock();
    if(parentPtr == newParent) {
        return;
    }

    TransformData transform = getRelativeTransform();
    if(keepGlobal) { transform = getGlobalTransform(); }

    if(parentPtr) {
        parent = std::weak_ptr<Transform>();
    }

    // We already handled this when parent was valid.
    if(!newParent) {
        return;
    }

    std::shared_ptr<Transform> curr = newParent;
    while(curr) {
        if(curr.get() == this) {
            // If the ancestor is equal to this, the ancestry creates a cycle - invalid.
            return;
        }
        // Climb to the next ancestor.
        curr = curr->parent.lock();
    }

    // If we reach here, that means the parent is valid, so:
    parent = newParent;

    if(keepGlobal) {
        setGlobalTransform(transform);
    } else {
        setRelativeTransform(transform);
    }
}

std::shared_ptr<Transform> Transform::getParent() const
{
    return parent.lock();
}

std::string to_string(const TransformData& data)
{
    std::stringstream ss;
    ss << "T(";
    ss << to_string(data.translation);
    ss << "), R(";
    ss << to_string(data.rotation);
    ss << "), S(";
    ss << to_string(data.scale);
    ss << ")";
    return ss.str();
}

TransformData& TransformData::operator*=(const TransformData& rhs)
{
    translation += rotation * (scale * rhs.translation);
    rotation *= rhs.rotation;
    scale *= rhs.scale;
    return *this;
}

TransformData TransformData::inverse() {
    TransformData inv;
    inv.rotation = glm::inverse(rotation);
    inv.scale = glm::vec3(
        scale.x == 0 ? 0 : (1.0f / scale.x),
        scale.y == 0 ? 0 : (1.0f / scale.y),
        scale.z == 0 ? 0 : (1.0f / scale.z)
    );
    inv.translation = -(inv.rotation * (inv.scale * translation));
    return inv;
}

glm::vec3 TransformData::transformPoint(const glm::vec3& point)
{
    return rotation * (scale * point) + translation;
}

glm::vec3 TransformData::transformDirection(const glm::vec3& direction)
{
    return rotation * direction;
}

glm::vec3 TransformData::transformDirectionWithScale(const glm::vec3& direction)
{
    return rotation * (scale * direction);
}

TransformData TransformData::lerp(TransformData other, float alpha) const
{
    TransformData result;
    result.translation = (1 - alpha) * translation + alpha * other.translation;
    result.rotation = slerp(rotation, other.rotation, alpha);
    result.scale = (1 - alpha) * scale + alpha * other.scale;
    return result;
}

glm::mat4 TransformData::toMat4()
{
    return glm::translate(glm::mat4(1.0f), translation)
        * glm::mat4_cast(rotation)
        * glm::scale(glm::mat4(1.0f), scale);
}

void Transform::setRelativeTransform(const TransformData& relativeTransform)
{
    this->relativeTransform = relativeTransform;
}

TransformData Transform::getGlobalTransform() const
{
    TransformData currTransform = relativeTransform;
    std::shared_ptr<Transform> curr = parent.lock();

    while(curr) {
        currTransform = curr->relativeTransform * currTransform;

        curr = curr->parent.lock();
    }
    return currTransform;
}

void Transform::setGlobalTransform(const TransformData& globalTransform)
{
    TransformData parentGlobal;
    std::shared_ptr<Transform> par = parent.lock();
    if(par) {
        parentGlobal = par->getGlobalTransform();
    }
    setRelativeTransform(parentGlobal.inverse() * globalTransform);
}

TransformData Transform::getTransformRelativeTo(std::shared_ptr<Transform> relative) const
{
    if(!relative) {
        return getGlobalTransform();
    }
    if(relative.get() == this) {
        return TransformData();
    }
    TransformData currTransform = relativeTransform;
    std::shared_ptr<Transform> curr = parent.lock();

    while(curr) {
        if(curr == relative) {
            return currTransform;
        }
        currTransform = curr->relativeTransform * currTransform;

        curr = curr->parent.lock();
    }
    return relative->getGlobalTransform().inverse() * currTransform;
}

void Transform::setTransformRelativeTo(const TransformData& transform, std::shared_ptr<Transform> relative)
{
    if(!relative) {
        setGlobalTransform(transform);
        return;
    }
    if(relative.get() == this) {
        return;
    }
    TransformData parentRelative;
    std::shared_ptr<Transform> par = parent.lock();
    if(par) {
        parentRelative = par->getTransformRelativeTo(relative).inverse();
    } else {
        parentRelative = relative->getGlobalTransform();
    }
    setRelativeTransform(parentRelative * transform);
}

