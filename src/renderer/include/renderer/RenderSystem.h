
#pragma once

#include "std.h"

#include "core/System.h"
#include "RenderSurface.h"

class RenderSystem : public System
{
    virtual void init() override;
    virtual void frameTick(float delta) override;
public:
    RenderSurface* targetSurface;
    bool swapBuffers = true;
};
