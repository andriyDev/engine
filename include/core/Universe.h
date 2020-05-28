
#pragma once

#include "std.h"

class Entity;
class Component;
class World;

class Universe
{
public:
    // The rate at which gameplay ticks should be issued (ticks / second).
    float gameplayRate = 50;
    // The maximum number of gameplay ticks that can be issued before we must skip.
    int maxGameplayTicksPerFrame = 10;

    /*
    Process ticking based on the delta time. Gameplay ticks are issued at a fixed rate,
    frame ticks are issued once per tick. Delta time is in seconds.
    */
    void tick(float deltaTime);

    /*
    Creates and adds a default world to the universe.
    Do not store shared_ptrs to this world afterwards.
    */
    std::shared_ptr<World> addWorld();

    /*
    Adds the world to the universe. This should be treated as a transfer of ownership.
    Do not store shared_ptrs to this world afterwards.
    This assumes world is not already owned by this universe.
    */
    void addWorld(std::shared_ptr<World> world);

    /*
    Removes the world from this universe. This should be treated as a transfer of ownership.
    After removeWorld, you are free to add this world back to the universe or do whatever.
    */
    std::shared_ptr<World> removeWorld(std::weak_ptr<World> world);
private:
    std::vector<std::shared_ptr<World>> worlds; // The worlds that this universe owns.

    float totalTime = 0; // The total time ticked.
    float gameplayTime = 0; // The gameplay time that has been processed (including skipped time).
    float skippedTime = 0; // The total skipped time.
};
