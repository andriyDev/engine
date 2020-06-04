
#pragma once

#include "std.h"

#include <glm/glm.hpp>

class RenderSurface
{
public:
    virtual vec2 getSize() const = 0;

    virtual void swapBuffers() = 0;
};
