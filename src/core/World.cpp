
#include "core/World.h"
#include "core/Query.h"
#include "core/Entity.h"
#include "core/System.h"

World::~World()
{
    for(System* system : systems) {
        delete system;
    }
}

Query World::query()
{
    return Query(this);
}

World* World::attach(Entity* entity)
{
    assert(!entity->worldId);
    entities.insert(entity);
    entity->worldId = id;
    return this;
}

World* World::detach(Entity* entity)
{
    assert(entity->worldId == id);
    entities.erase(entity);
    entity->worldId = 0;
    return this;
}

World* World::addSystem(System* system)
{
    assert(!system->world);
    system->world = this;
    for(auto it = systems.begin(); it != systems.end(); it++) {
        if((*it)->priority <= system->priority) {
            systems.insert(it, system);
            return this;
        }
    }
    systems.push_back(system);
    return this;
}
