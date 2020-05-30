
#pragma once

#include "std.h"
#include "Query.h"

class Entity;
class System;

class World : public std::enable_shared_from_this<World>
{
public:
    // Returns a query that can filter down entities in the world.
    Query<std::shared_ptr<Entity>> queryEntities();
    // Returns a query that can filter down components in the world.
    Query<std::shared_ptr<Component>> queryComponents();
    
    /*
    Constructs an empty entity and returns a pointer to it.
    Do not store shared_ptrs to the entity afterwards.
    */
    std::shared_ptr<Entity> addEntity();
    /* Transfers ownership of an entity to this world. */
    void addEntity(std::shared_ptr<Entity> entity);
    /* Removes the entity from the world's ownership. */
    void removeEntity(std::shared_ptr<Entity> entity);

    /*
    Adds the system in order of decreasing priority (if priority is equal, later additions will be first).
    Do not store shared_ptrs to the system afterwards.
    */
    void addSystem(std::shared_ptr<System> system);
    // Creates the system as the provided type, and performs just as addSystem does.
    template<class T>
    std::shared_ptr<T> addSystem(float priority = 0)
    {
        std::shared_ptr<T> newSystem = std::make_shared<T>();
        newSystem->priority = priority;
        addSystem(newSystem);
        return newSystem;
    }

    /*
    Ticks once per frame.
    delta is the time in seconds since the last frame.
    tickPercent is in [0,1] and says how far we are into the current tick.
    */
    void frameTick(float delta);
    /*
    Ticks once per physics rate.
    delta is the phyics rate.
    */
    void gameplayTick(float delta);

    inline std::set<std::shared_ptr<Entity>> getEntities() const {
        return entities;
    }
    inline std::vector<std::shared_ptr<System>> getSystems() const {
        return systems;
    }
private:
    std::set<std::shared_ptr<Entity>> entities;
    std::vector<std::shared_ptr<System>> systems;

    friend class Universe;
};
