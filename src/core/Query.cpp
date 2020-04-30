
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
        std::set<Component*> components = entity->getComponents();
        items.insert(components.begin(), components.end());
    }
}

std::set<uint> toIdSet(const Query<Entity*>& query)
{
    std::set<uint> results;
    for(Entity* entity : query) {
        results.insert(entity->getId());
    }
    return results;
}

std::set<uint> toIdSet(const Query<Component*>& query)
{
    std::set<uint> results;
    for(Component* component : query) {
        results.insert(component->getId());
    }
    return results;
}

std::function<bool(Component*)> filterByTypeId(uint typeId)
{
    return [typeId](Component* C) { return C->getTypeId() == typeId; };
}
