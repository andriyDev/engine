
#include "core/World.h"
#include "core/Query.h"
#include "core/Entity.h"
#include "core/Component.h"
#include "core/System.h"

Query<std::shared_ptr<Entity>> World::queryEntities()
{
    return Query<std::shared_ptr<Entity>>(entities);
}

Query<std::shared_ptr<Component>> World::queryComponents(uint type)
{
    auto it = components.find(type);
    return Query<std::shared_ptr<Component>>(
        it == components.end() ? std::unordered_set<std::shared_ptr<Component>>() : it->second);
}

std::shared_ptr<Entity> World::addEntity()
{
    std::shared_ptr<Entity> ptr = std::make_shared<Entity>();
    addEntity(ptr);
    return ptr;
}

void World::removeEntity(std::shared_ptr<Entity> entity)
{
    entities.erase(entity);
    entity->world = std::shared_ptr<World>();
    for(const std::shared_ptr<Component>& child : entity->components) {
        removeComponent(child);
    }
}

void World::addEntity(std::shared_ptr<Entity> entity)
{
    entities.insert(entity);
    entity->world = shared_from_this();
    for(const std::shared_ptr<Component>& child : entity->components) {
        addComponent(child);
    }
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

void World::frameTick(float delta)
{
    for(std::shared_ptr<System> system : systems) {
        if(!system->initialized) {
            system->initialized = true;
            system->init();
        }
        system->frameTick(delta);
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

void World::addComponent(std::shared_ptr<Component> component)
{
    auto pair = components.insert(std::make_pair(component->getTypeId(),
        std::unordered_set<std::shared_ptr<Component>>()));
    pair.first->second.insert(component);
}

void World::removeComponent(std::shared_ptr<Component> component)
{
    auto it = components.find(component->getTypeId());
    if(it != components.end()) {
        it->second.erase(component);
    }
}