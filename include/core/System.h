
#pragma once

#include "std.h"

class System
{
public:
    /*
    Ticks once per frame.
    delta is the time in seconds since the last frame.
    tickPercent is in [0,1] and says how far we are into the current tick.
    */
    virtual void frameTick(float delta, float tickPercent) {}
    /*
    Ticks once per physics rate.
    delta is the phyics rate.
    */
    virtual void gameplayTick(float delta) {}
};
