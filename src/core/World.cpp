
#include "core/World.h"
#include "core/Query.h"
#include "core/Entity.h"
#include "core/System.h"

Query<Entity*> World::queryEntities()
{
    return Query<Entity*>(this);
}

Query<Component*> World::queryComponents()
{
    return Query<Component*>(this);
}

std::shared_ptr<Entity> World::addEntity()
{
    std::shared_ptr<Entity> ptr = std::make_shared<Entity>();
    addEntity(ptr);
    return ptr;
}

void World::addEntity(std::shared_ptr<Entity> entity)
{
    entities.insert(entity);
    entity->world = shared_from_this();
}

void World::addSystem(std::shared_ptr<System> system)
{
    system->world = shared_from_this();
    for(auto it = systems.begin(); it != systems.end(); it++) {
        if((*it)->priority <= system->priority) {
            systems.insert(it, system);
            return;
        }
    }
    systems.push_back(system);
}

void World::frameTick(float delta, float tickPercent)
{
    for(std::shared_ptr<System> system : systems) {
        if(!system->initialized) {
            system->initialized = true;
            system->init();
        }
        system->frameTick(delta, tickPercent);
    }
}

void World::gameplayTick(float delta)
{
    for(std::shared_ptr<System> system : systems) {
        if(!system->initialized) {
            system->initialized = true;
            system->init();
        }
        system->gameplayTick(delta);
    }
}
