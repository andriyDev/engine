
#pragma once

#include "std.h"

class Entity;
class Component;
class World;

class Universe
{
public:
    // Initializes the Universe singleton. Also returns the pointer to it.
    static Universe* init();
    // Deletes the Universe singleton.
    static void cleanUp();
    // Gets a pointer to the Universe singleton.
    static Universe* get();

    ~Universe();

    // The rate at which gameplay ticks should be issued (ticks / second).
    float gameplayRate = 1.0f / 50;
    // The maximum number of gameplay ticks that can be issued before we must skip.
    int maxGameplayTicksPerFrame = 10;

    // Assigns the entity an id and stores it. Returns the entity pointer.
    Entity* addEntity(Entity* entity);
    // Assigns the component an id and stores it. Returns the component pointer.
    Component* addComponent(Component* component);
    // Assigns the world an id and stores it. Returns the world pointer.
    World* addWorld(World* world);
    
    // Removes and deletes the entity. Also removes attached components iff removeDependent = true
    void removeEntity(Entity* entity, bool removeDependent);
    // Removes and deletes the component.
    void removeComponent(Component* component);
    // Removes and deletes the world. Also removes attached components/entities iff removeDependent = true
    void removeWorld(World* world, bool removeDependent);

    // Process ticking based on the delta time. Gameplay ticks are issued at a fixed rate, frame ticks are issued once per tick. Delta time is in seconds.
    void tick(float deltaTime);

    Entity* getEntity(uint id) const {
        auto f = entities.find(id);
        return f != entities.end() ? f->second : nullptr;
    }
    Component* getComponent(uint id) const {
        auto f = components.find(id);
        return f != components.end() ? f->second : nullptr;
    }
    World* getWorld(uint id) const {
        auto f = worlds.find(id);
        return f != worlds.end() ? f->second : nullptr;
    }
private:
    map<uint, Entity*> entities; // The set of all entities mapped by their ids.
    map<uint, Component*> components; // The set of all components mapped by their ids.
    map<uint, World*> worlds; // The set of all worlds mapped by their ids.

    float totalTime = 0; // The total time ticked.
    float gameplayTime = 0; // The gameplay time that has been processed (including skipped time).
    float skippedTime = 0; // The total skipped time.

    static Universe* U;
};
