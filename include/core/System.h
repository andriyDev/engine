
#pragma once

#include "std.h"

class System
{
public:
    /*
    The priority of this system. Specifically, the higher the number, the earlier in the tick it will be called.
    */
    float priority = 0;
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
