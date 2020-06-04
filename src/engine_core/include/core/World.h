
#pragma once

#include "std.h"
#include "Query.h"

class Entity;
class System;

class World : public enable_shared_from_this<World>
{
public:
    // Returns a query that can filter down entities in the world.
    Query<shared_ptr<Entity>> queryEntities();
    // Returns a query that can filter down components in the world.
    Query<shared_ptr<Component>> queryComponents(uint type);
    
    /*
    Constructs an empty entity and returns a pointer to it.
    Do not store shared_ptrs to the entity afterwards.
    */
    shared_ptr<Entity> addEntity();
    /* Transfers ownership of an entity to this world. */
    void addEntity(shared_ptr<Entity> entity);
    /* Removes the entity from the world's ownership. */
    void removeEntity(shared_ptr<Entity> entity);

    /*
    Adds the system in order of decreasing priority (if priority is equal, later additions will be first).
    Do not store shared_ptrs to the system afterwards.
    */
    void addSystem(shared_ptr<System> system);
    // Creates the system as the provided type, and performs just as addSystem does.
    template<class T>
    shared_ptr<T> addSystem(float priority = 0)
    {
        shared_ptr<T> newSystem = make_shared<T>();
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

    void addComponent(shared_ptr<Component> component);
    void removeComponent(shared_ptr<Component> component);

    inline hash_set<shared_ptr<Entity>> getEntities() const {
        return entities;
    }
    inline vector<shared_ptr<System>> getSystems() const {
        return systems;
    }
private:
    hash_set<shared_ptr<Entity>> entities;
    hash_map<uint, hash_set<shared_ptr<Component>>> components;
    vector<shared_ptr<System>> systems;

    friend class Universe;
    friend class Entity;
};
