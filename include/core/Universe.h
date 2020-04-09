
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

    // Assigns the entity an id and stores it. Returns the entity pointer.
    Entity* registerEntity(Entity* entity);
    // Assigns the component an id and stores it. Returns the component pointer.
    Component* registerComponent(Component* component);
    // Assigns the world an id and stores it. Returns the world pointer.
    World* registerWorld(World* world);

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

    static Universe* U;
};
