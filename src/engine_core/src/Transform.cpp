
#include "components/Transform.h"
#include "core/Entity.h"
#include "core/Universe.h"

#include <sstream>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/transform.hpp>

Transform::~Transform()
{
    shared_ptr<Transform> parentPtr = parent.lock();
    if(parentPtr) {
        // Erase one null pointer since that will correspond to us.
        auto it = find_if(parentPtr->children.begin(), parentPtr->children.end(),
            [](const weak_ptr<Transform>& child){
                return child.lock() == nullptr;
            });
        assert(it != parentPtr->children.end());
        parentPtr->children.erase(it);
    }
}

void Transform::setParent(shared_ptr<Transform> newParent, bool keepGlobal)
{
    shared_ptr<Transform> parentPtr = parent.lock();
    if(parentPtr == newParent) {
        return;
    }

    TransformData transform = getRelativeTransform();
    if(keepGlobal) { transform = getGlobalTransform(); }

    shared_ptr<Transform> sharedThis = static_pointer_cast<Transform>(shared_from_this());

    if(parentPtr) {
        parent = weak_ptr<Transform>();

        auto it = find_if(parentPtr->children.begin(), parentPtr->children.end(),
            [&sharedThis](const weak_ptr<Transform>& child){
                return sharedThis == child.lock();
            });
        assert(it != parentPtr->children.end());
        parentPtr->children.erase(it);
    }

    // We put this a little higher because otherwise the compiler complains about the goto.
    shared_ptr<Transform> curr = newParent;
    // We already handled this when parent was valid.
    if(!newParent) {
        goto end; // goto here is convenient because we also need to goto in the case of a cycle.
    }

    while(curr) {
        if(curr.get() == this) {
            // If the ancestor is equal to this, the ancestry creates a cycle - invalid (attaches to world).
            goto end;
        }
        // Climb to the next ancestor.
        curr = curr->parent.lock();
    }

    // If we reach here, that means the parent is valid, so:
    parent = newParent;
    // Tell the parent we are a child.
    newParent->children.push_back(sharedThis);
end:
    // Setting global transform changes the update id.
    if(keepGlobal) {
        setGlobalTransform(transform);
    } else { // If we don't set the global transform, just increment.
        incrementUpdateId();
    }
}

shared_ptr<Transform> Transform::getParent() const
{
    return parent.lock();
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

TransformData TransformData::inverse() const
{
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

glm::vec3 TransformData::transformPoint(const glm::vec3& point) const
{
    return rotation * (scale * point) + translation;
}

glm::vec3 TransformData::transformDirection(const glm::vec3& direction) const
{
    return rotation * direction;
}

glm::vec3 TransformData::transformDirectionWithScale(const glm::vec3& direction) const
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

glm::mat4 TransformData::toMat4() const
{
    return glm::translate(glm::mat4(1.0f), translation)
        * glm::mat4_cast(rotation)
        * glm::scale(glm::mat4(1.0f), scale);
}

void Transform::setRelativeTransform(const TransformData& relativeTransform)
{
    this->relativeTransform = relativeTransform;
    incrementUpdateId();
}

void Transform::incrementUpdateId()
{
    updateId++;
}

uint Transform::sumUpdatesRelativeTo(shared_ptr<Transform> relative) const
{
    if(relative.get() == this) {
        return 0;
    }
    uint sum = updateId;
    shared_ptr<Transform> curr = parent.lock();

    while(curr && curr != relative) {
        sum += curr->updateId;

        curr = curr->parent.lock();
    }
    return sum;
}

TransformData Transform::getGlobalTransform() const
{
    TransformData currTransform = relativeTransform;
    shared_ptr<Transform> curr = parent.lock();

    while(curr) {
        currTransform = curr->relativeTransform * currTransform;

        curr = curr->parent.lock();
    }
    return currTransform;
}

void Transform::setGlobalTransform(const TransformData& globalTransform)
{
    TransformData parentGlobal;
    shared_ptr<Transform> par = parent.lock();
    if(par) {
        parentGlobal = par->getGlobalTransform();
    }
    setRelativeTransform(parentGlobal.inverse() * globalTransform);
    // setRelativeTransform changes the updateId for us, so we don't have to.
}

TransformData Transform::getTransformRelativeTo(shared_ptr<Transform> relative) const
{
    if(!relative) {
        return getGlobalTransform();
    }
    if(relative.get() == this) {
        return TransformData();
    }
    TransformData currTransform = relativeTransform;
    shared_ptr<Transform> curr = parent.lock();

    while(curr) {
        if(curr == relative) {
            return currTransform;
        }
        currTransform = curr->relativeTransform * currTransform;

        curr = curr->parent.lock();
    }
    return relative->getGlobalTransform().inverse() * currTransform;
}

void Transform::setTransformRelativeTo(const TransformData& transform, shared_ptr<Transform> relative)
{
    if(!relative) {
        setGlobalTransform(transform);
        return;
    }
    if(relative.get() == this) {
        return;
    }
    TransformData parentRelative;
    shared_ptr<Transform> par = parent.lock();
    if(par) {
        parentRelative = par->getTransformRelativeTo(relative).inverse();
    } else {
        parentRelative = relative->getGlobalTransform();
    }
    setRelativeTransform(parentRelative * transform);
    // setRelativeTransform changes the updateId for us, so we don't have to.
}

vector<shared_ptr<Transform>> Transform::getChildren() const
{
    vector<shared_ptr<Transform>> out;

    for(const weak_ptr<Transform>& childPtr : children) {
        shared_ptr<Transform> child = childPtr.lock();
        if(child) {
            out.push_back(child);
        }
    }

    return out;
}

shared_ptr<Transform> mapToTransform(shared_ptr<Transformable> component)
{
    return component ? component->getTransform() : nullptr;
}
