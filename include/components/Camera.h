
#pragma once

#include "std.h"
#include "core/Component.h"

#include "ComponentTypes.h"

#include <glm/glm.hpp>

using namespace glm;

class Camera : public Component
{
public:
    float fov = 90;
    float nearClip = 0.01f;
    float farClip = 10000;
    float aspect = 0;

    Camera() : Component(CAMERA_ID)
    {}

    mat4 getProjectionMatrix(float surfaceAspect) const;

    mat4 getViewMatrix(float interpolation) const;

    mat4 getVPMatrix(float interpolation, float surfaceAspect) const;
};
