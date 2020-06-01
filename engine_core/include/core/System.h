
#pragma once

#include "std.h"

class World;

class System
{
public:
    /*
    The priority of this system. Specifically, the higher the number, the earlier in the tick it will be called.
    */
    float priority = 0;
    /* Called on the first frame that the system runs. Allows the system to initialize its data. */
    virtual void init() {}
    /*
    Ticks once per frame.
    delta is the time in seconds since the last frame.
    */
    virtual void frameTick(float delta) {}
    /*
    Ticks once per physics rate.
    delta is the phyics rate.
    */
    virtual void gameplayTick(float delta) {}

    inline std::shared_ptr<World> getWorld() const {
        return world.lock();
    }
private:
    std::weak_ptr<World> world; // The world that this system manages.
    bool initialized = false; // Has this system been initialized.

    friend class World;
};
