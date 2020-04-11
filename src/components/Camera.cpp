
#include "components/Camera.h"

#include "core/Universe.h"
#include "core/Entity.h"

#include "components/Transform.h"

mat4 Camera::getProjectionMatrix(float surfaceAspect) const
{
    return perspective(radians(fov), aspect == 0 ? surfaceAspect : aspect, nearClip, farClip);
}

mat4 Camera::getViewMatrix() const
{
    Transform* t = Transform::getComponentTransform(this);
    TransformData td = t ? t->getGlobalTransform().inverse() : TransformData();
    return td.toMat4();
}

mat4 Camera::getVPMatrix(float surfaceAspect) const
{
    return getProjectionMatrix(surfaceAspect) * getViewMatrix();
}
