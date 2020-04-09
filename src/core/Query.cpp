
#include "core/Query.h"
#include "core/Component.h"
#include "core/Entity.h"
#include "core/World.h"
#include "core/Universe.h"

#include <algorithm>
#include <iterator>

Query& Query::filter(uint componentTypeId, inclusivity inverted)
{
    set<uint> filtered;
    for(uint entityId : entityIds) {
        Entity* entity = universe->getEntity(entityId);
        assert(entity);
        bool hasComponent = false;
        for(uint componentId : entity->getComponentIds()) {
            Component* component = universe->getComponent(componentId);
            assert(component);
            if(component->getTypeId() == componentId) {
                hasComponent = true;
                break;
            }
        }
        if(hasComponent) {
            filtered.insert(entityId);
        }
    }
    if(inverted == INCLUSIVE) {
        entityIds = move(filtered);
    } else {
        set<uint> diff;
        set_difference(
            entityIds.begin(), entityIds.end(),
            filtered.begin(), filtered.end(),
            inserter(diff, diff.end())
        );
        entityIds = move(diff);
    }
    return *this;
}

Query& Query::operator|=(const Query& other)
{
    entityIds.insert(other.entityIds.begin(), other.entityIds.end());
    return *this;
}

Query& Query::operator&=(const Query& other)
{
    set<uint> result;

    set_intersection(
        entityIds.begin(), entityIds.end(),
        other.entityIds.begin(), other.entityIds.end(),
        inserter(result, result.end())
    );
    entityIds = move(result);
    return *this;
}

Query& Query::operator-=(const Query& other)
{
    set<uint> result;

    set_difference(
        entityIds.begin(), entityIds.end(),
        other.entityIds.begin(), other.entityIds.end(),
        inserter(result, result.end())
    );
    entityIds = move(result);
    return *this;
}

Query& Query::operator~()
{
    set<uint> result;
    set<uint> allEntities = world->getEntities();

    set_difference(
        allEntities.begin(), allEntities.end(),
        entityIds.begin(), entityIds.end(),
        inserter(result, result.end())
    );
    entityIds = move(result);
    return *this;
}
