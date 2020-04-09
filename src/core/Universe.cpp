
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

Entity* Universe::registerEntity(Entity* entity)
{
    uint id = generateUnusedId(entities);
    entity->id = id;
    entities.insert(make_pair(id, entity));
    return entity;
}

Component* Universe::registerComponent(Component* component)
{
    uint id = generateUnusedId(components);
    component->id = id;
    components.insert(make_pair(id, component));
    return component;
}

World* Universe::registerWorld(World* world)
{
    uint id = generateUnusedId(worlds);
    world->id = id;
    worlds.insert(make_pair(id, world));
    return world;
}
