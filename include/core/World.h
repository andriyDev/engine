
#pragma once

#include "std.h"

class Query;
class Entity;
class System;

class World
{
public:
    ~World();

    // Returns a query that can filter down entities in the world.
    Query query();

    // Attaches the entity to this world. Fails if the entity is attached to another world.
    World* attach(Entity* entity);
    // Detaches the entity from this world. Fails if the entity is not in this world.
    World* detach(Entity* entity);

    // Adds the system in order of decreasing priority (if priority is equal, later additions will be first).
    World* addSystem(System* system);

    inline uint getId() const {
        return id;
    }
    inline set<Entity*> getEntities() const {
        return entities;
    }
private:
    uint id;
    set<Entity*> entities;
    vector<System*> systems;

    friend class Universe;
};
