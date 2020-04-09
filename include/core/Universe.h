
#pragma once

#include "std.h"

class Entity;
class Component;
class World;

class Universe
{
public:
    Entity* registerEntity(Entity* entity);
    Component* registerComponent(Component* component);

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
    map<uint, Entity*> entities;
    map<uint, Component*> components;
    map<uint, World*> worlds;
};
