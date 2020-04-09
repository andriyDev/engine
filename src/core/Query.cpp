
#include "core/Query.h"
#include "core/Component.h"
#include "core/Entity.h"
#include "core/World.h"
#include "core/Universe.h"

#include <algorithm>
#include <iterator>

Query::Query(World* world)
{
    entities = world->getEntities();
}

Query& Query::filter(uint componentTypeId, inclusivity inverted)
{
    set<Entity*> filtered;
    for(Entity* entity : entities) {
        bool hasComponent = false;
        for(Component* component : entity->getComponents()) {
            if(component->getTypeId() == componentTypeId) {
                hasComponent = true;
                break;
            }
        }
        if(hasComponent) {
            filtered.insert(entity);
        }
    }
    if(inverted == INCLUSIVE) {
        entities = move(filtered);
    } else {
        set<Entity*> diff;
        set_difference(
            entities.begin(), entities.end(),
            filtered.begin(), filtered.end(),
            inserter(diff, diff.end())
        );
        entities = move(diff);
    }
    return *this;
}

set<uint> Query::toIdSet() const
{
    set<uint> ids;
    for(Entity* entity : entities) {
        ids.insert(entity->getId());
    }
    return ids;
}

Query& Query::operator|=(const Query& other)
{
    entities.insert(other.entities.begin(), other.entities.end());
    return *this;
}

Query& Query::operator&=(const Query& other)
{
    set<Entity*> result;

    set_intersection(
        entities.begin(), entities.end(),
        other.entities.begin(), other.entities.end(),
        inserter(result, result.end())
    );
    entities = move(result);
    return *this;
}

Query& Query::operator-=(const Query& other)
{
    set<Entity*> result;

    set_difference(
        entities.begin(), entities.end(),
        other.entities.begin(), other.entities.end(),
        inserter(result, result.end())
    );
    entities = move(result);
    return *this;
}

Query& Query::operator~()
{
    set<Entity*> result;
    Query all(world);

    set_difference(
        all.entities.begin(), all.entities.end(),
        entities.begin(), entities.end(),
        inserter(result, result.end())
    );
    entities = move(result);
    return *this;
}
