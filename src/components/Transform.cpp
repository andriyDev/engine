
#include "components/Transform.h"
#include "core/Entity.h"

#include <sstream>
#include <glm/gtx/string_cast.hpp>

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

string to_string(const TransformData& data)
{
    stringstream ss;
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
    inv.scale = vec3(
        scale.x == 0 ? 0 : (1.0f / scale.x),
        scale.y == 0 ? 0 : (1.0f / scale.y),
        scale.z == 0 ? 0 : (1.0f / scale.z)
    );
    inv.translation = -(inv.rotation * (inv.scale * translation));
    return inv;
}

vec3 TransformData::transformPoint(const vec3& point)
{
    return rotation * (scale * point) + translation;
}

vec3 TransformData::transformDirection(const vec3& direction)
{
    return rotation * direction;
}

vec3 TransformData::transformDirectionWithScale(const vec3& direction)
{
    return rotation * (scale * direction);
}

TransformData TransformData::lerp(TransformData& other, float alpha)
{
    TransformData result;
    result.translation = (1 - alpha) * translation + alpha * other.translation;
    result.rotation = slerp(rotation, other.rotation, alpha);
    result.scale = (1 - alpha) * scale + alpha * other.scale;
    return result;
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

void Transform::setGlobalTransform(const TransformData& globalTransform, bool teleport)
{
    TransformData parentGlobal;
    if(parent) {
        parentGlobal = parent->getGlobalTransform();
    }
    setRelativeTransform(parentGlobal.inverse() * globalTransform, teleport);
}
