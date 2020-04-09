
#include "core/Universe.h"
#include "core/Entity.h"
#include "core/Component.h"
#include "core/World.h"

Universe* Universe::U = nullptr;

Universe* Universe::init()
{
    Universe::U = new Universe();
    return Universe::U;
}

void Universe::cleanUp()
{
    delete Universe::U;
    Universe::U = nullptr;
}

Universe* Universe::get()
{
    return Universe::U;
}

Universe::~Universe()
{
    for(auto p : entities) {
        delete p.second;
    }
    for(auto p : components) {
        delete p.second;
    }
    for(auto p : worlds) {
        delete p.second;
    }
}

uint generateUniqueId()
{
    uint r = rand();
    return r == 0 ? 1 : r;
}

template<typename V>
uint generateUnusedId(map<uint, V>& space)
{
    uint id;
    do
    {
        id = generateUniqueId();
    } while(space.count(id));
    return id;
}

Entity* Universe::addEntity(Entity* entity)
{
    uint id = generateUnusedId(entities);
    entity->id = id;
    entities.insert(make_pair(id, entity));
    return entity;
}

Component* Universe::addComponent(Component* component)
{
    uint id = generateUnusedId(components);
    component->id = id;
    components.insert(make_pair(id, component));
    return component;
}

World* Universe::addWorld(World* world)
{
    uint id = generateUnusedId(worlds);
    world->id = id;
    worlds.insert(make_pair(id, world));
    return world;
}

void Universe::removeEntity(Entity* entity, bool removeDependent)
{
    assert(entity->id);
    entities.erase(entity->id);
    if(entity->worldId) {
        World* world = getWorld(entity->worldId);
        assert(world);
        world->detach(entity);
    }
    if(removeDependent) {
        for(Component* component : entity->components) {
            component->ownerId = 0;
            removeComponent(component);
        }
    }
    delete entity;
}

void Universe::removeComponent(Component* component)
{
    assert(component->id);
    components.erase(component->id);
    if(component->ownerId) {
        Entity* entity = getEntity(component->ownerId);
        assert(entity);
        entity->detach(component);
    }
    delete component;
}

void Universe::removeWorld(World* world, bool removeDependent)
{
    assert(world->id);
    worlds.erase(world->id);
    if(removeDependent) {
        for(Entity* entity : world->entities) {
            entity->worldId = 0;
            removeEntity(entity, true);
        }
    }
    delete world;
}

void Universe::tick(float deltaTime)
{
    totalTime += deltaTime;
    float gameplayDelta = 1.0f / gameplayRate;
    float remainingGameplayTime = totalTime - gameplayTime;
    int desiredGameplayFrames = (int)(remainingGameplayTime * gameplayRate);
    int issuedGameplayFrames = desiredGameplayFrames < maxGameplayTicksPerFrame
        ? desiredGameplayFrames : maxGameplayTicksPerFrame;

    for(int i = 0; i < issuedGameplayFrames; i++) {
        gameplayTime += gameplayDelta;
        for(auto p : worlds) {
            p.second->gameplayTick(gameplayDelta);
        }
    }

    float currentSkippedTime = (desiredGameplayFrames - issuedGameplayFrames) * gameplayRate;
    skippedTime += currentSkippedTime;
    gameplayTime += currentSkippedTime;

    float tickPercent = (totalTime - gameplayTime) * gameplayRate;
    for(auto p : worlds) {
        p.second->frameTick(deltaTime, tickPercent);
    }
}
