
#pragma once

#include "std.h"

#include "core/System.h"
#include "Window.h"

class RenderSystem : public System
{
    virtual void init() override;
    virtual void frameTick(float delta) override;
public:
    Window* targetWindow;
};
