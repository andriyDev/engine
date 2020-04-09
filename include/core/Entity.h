
#pragma once

#include "std.h"

class Component;

class Entity
{
public:
    // Attaches the provided component to this entity.
    Entity* attach(Component* component);

    inline uint getId() const {
        return id;
    }
    inline set<uint> getComponentIds() const {
        return components;
    }
private:
    uint id = 0; // The id of the entity.
    set<uint> components; // The components attached to this entity.

    friend class Universe;
};
