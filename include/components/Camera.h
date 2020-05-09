
#pragma once

#include "std.h"
#include "components/Transform.h"

#include "ComponentTypes.h"

#include <glm/glm.hpp>

class Camera : public Transformable
{
public:
    float fov = 90;
    float nearClip = 0.01f;
    float farClip = 10000;
    float aspect = 0;

    Camera() : Transformable(CAMERA_ID) {}

    glm::mat4 getProjectionMatrix(float surfaceAspect) const;

    glm::mat4 getViewMatrix(float interpolation) const;

    glm::mat4 getVPMatrix(float interpolation, float surfaceAspect) const;
};
