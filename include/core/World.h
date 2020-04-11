
#pragma once

#include "std.h"
#include "Query.h"

class Entity;
class System;

class World
{
public:
    ~World();

    // Returns a query that can filter down entities in the world.
    Query<Entity*> queryEntities();
    // Returns a query that can filter down components in the world.
    Query<Component*> queryComponents();

    // Attaches the entity to this world. Fails if the entity is attached to another world.
    World* attach(Entity* entity);
    // Detaches the entity from this world. Fails if the entity is not in this world.
    World* detach(Entity* entity);

    // Adds the system in order of decreasing priority (if priority is equal, later additions will be first).
    World* addSystem(System* system);
    // Creates the system as the provided type, and performs just as addSystem does.
    template<class T>
    World* addSystem(float priority = 0)
    {
        System* newSystem = new T();
        newSystem->priority = priority;
        return addSystem(newSystem);
    }

    /*
    Ticks once per frame.
    delta is the time in seconds since the last frame.
    tickPercent is in [0,1] and says how far we are into the current tick.
    */
    void frameTick(float delta, float tickPercent);
    /*
    Ticks once per physics rate.
    delta is the phyics rate.
    */
    void gameplayTick(float delta);

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
