
#pragma once

#include "std.h"

class Component;

class Entity
{
public:
    // Attaches the provided component to this entity. Fails is the component is attached to another entity.
    Entity* attach(Component* component);
    // Detaches the provided component from this entity. Fails if the component was not attached.
    Entity* detach(Component* component);

    // Finds any component attached to this entity with the specified type.
    Component* findComponentByType(int typeId);
    // Finds all components attached to this entity with the specified type.
    set<Component*> findComponentsByType(int typeId);

    inline uint getId() const {
        return id;
    }
    inline uint getWorldId() const {
        return worldId;
    }
    inline set<Component*> getComponents() const {
        return components;
    }
private:
    uint id = 0; // The id of the entity.
    uint worldId = 0; // The id of the world this entity is in.
    set<Component*> components; // The components attached to this entity.

    friend class Universe;
    friend class World;
};
