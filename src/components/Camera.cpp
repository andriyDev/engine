
#include "components/Camera.h"

#include "core/Universe.h"
#include "core/Entity.h"

#include "components/Transform.h"

glm::mat4 Camera::getProjectionMatrix(float surfaceAspect) const
{
    return glm::perspective(glm::radians(fov), aspect == 0 ? surfaceAspect : aspect, nearClip, farClip);
}

glm::mat4 Camera::getViewMatrix(float interpolation) const
{
    std::shared_ptr<Transform> t = getTransform();
    if(t) {
        TransformData td = t ? t->getGlobalTransform(interpolation).inverse() : TransformData();
        return td.toMat4();
    } else {
        return TransformData().toMat4();
    }
}

glm::mat4 Camera::getVPMatrix(float interpolation, float surfaceAspect) const
{
    return getProjectionMatrix(surfaceAspect) * getViewMatrix(interpolation);
}
