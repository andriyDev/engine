
#include "components/Transform.h"
#include "core/Entity.h"
#include "core/Universe.h"

#include <sstream>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/transform.hpp>

void Transform::setParent(Transform* newParent, bool keepGlobal)
{
    if(parent == newParent) {
        return;
    }

    TransformData transform = getRelativeTransform();
    if(keepGlobal) { transform = getGlobalTransform(); }

    if(parent) {
        parent = nullptr;
    }

    // We already handled this when parent was valid.
    if(!newParent) {
        return;
    }

    Transform* curr = newParent;
    while(curr) {
        if(curr == this) {
            // If the ancestor is equal to this, the ancestry creates a cycle - invalid.
            return;
        }
        // Climb to the next ancestor.
        curr = curr->parent;
    }

    // If we reach here, that means the parent is valid, so:
    parent = newParent;

    if(keepGlobal) {
        setGlobalTransform(transform, true);
    } else {
        setRelativeTransform(transform, true);
    }
}

Transform* Transform::getParent() const
{
    return parent;
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

void Transform::setRelativeTransform(const TransformData& relativeTransform, bool teleport)
{
    this->relativeTransform = relativeTransform;
    if(teleport) {
        previousTransform[0] = relativeTransform;
        previousTransform[1] = relativeTransform;
    }
}

TransformData Transform::getGlobalTransform() const
{
    TransformData currTransform;
    Transform* curr = const_cast<Transform*>(this);

    while(curr) {
        currTransform = curr->relativeTransform * currTransform;

        curr = curr->parent;
    }
    return currTransform;
}

TransformData Transform::getGlobalTransform(float interpolation) const
{
    TransformData currTransform;
    Transform const* curr = this;

    while(curr) {
        currTransform = curr->getRelativeTransform(interpolation) * currTransform;

        curr = curr->parent;
    }
    return currTransform;
}

void Transform::setGlobalTransform(const TransformData& globalTransform, bool teleport)
{
    TransformData parentGlobal;
    if(parent) {
        parentGlobal = parent->getGlobalTransform();
    }
    setRelativeTransform(parentGlobal.inverse() * globalTransform, teleport);
}

Transform* Transform::getComponentTransform(Component const* comp)
{
    Entity* owner = Universe::get()->getEntity(comp->getOwnerId());
    return static_cast<Transform*>(owner->findComponentByType(TRANSFORM_ID));
}
