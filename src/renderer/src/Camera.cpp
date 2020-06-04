
#include "renderer/Camera.h"

#include "core/Universe.h"
#include "core/Entity.h"

#include "components/Transform.h"

mat4 Camera::getProjectionMatrix(float surfaceAspect) const
{
    return perspective(radians(fov), aspect == 0 ? surfaceAspect : aspect, nearClip, farClip);
}

mat4 Camera::getViewMatrix() const
{
    shared_ptr<Transform> t = getTransform();
    if(t) {
        TransformData td = t ? t->getGlobalTransform().inverse() : TransformData();
        return td.toMat4();
    } else {
        return TransformData().toMat4();
    }
}

mat4 Camera::getVPMatrix(float surfaceAspect) const
{
    return getProjectionMatrix(surfaceAspect) * getViewMatrix();
}
