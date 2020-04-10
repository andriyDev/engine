
#pragma once

#include "std.h"

#include "core/System.h"

class RenderSystem : public System
{
    virtual void frameTick(float delta, float tickPercent) override;
    virtual void gameplayTick(float delta) override;
};
