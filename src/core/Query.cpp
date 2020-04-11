
#include "core/Query.h"
#include "core/Component.h"
#include "core/Entity.h"
#include "core/World.h"
#include "core/Universe.h"

#include <algorithm>
#include <iterator>

template<>
Query<Entity*>::Query(World* world)
{
    this->world = world;
    items = world->getEntities();
}

template<>
Query<Component*>::Query(World* world)
{
    this->world = world;
    for(Entity* entity : world->getEntities()) {
        set<Component*> components = entity->getComponents();
        items.insert(components.begin(), components.end());
    }
}

set<uint> toIdSet(const Query<Entity*>& query)
{
    set<uint> results;
    for(Entity* entity : query) {
        results.insert(entity->getId());
    }
    return results;
}

set<uint> toIdSet(const Query<Component*>& query)
{
    set<uint> results;
    for(Component* component : query) {
        results.insert(component->getId());
    }
    return results;
}

function<bool(Component*)> filterByTypeId(uint typeId)
{
    return [typeId](Component* C) { return C->getTypeId() == typeId; };
}
