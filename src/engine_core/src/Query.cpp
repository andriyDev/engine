
#include "core/Query.h"
#include "core/Component.h"
#include "core/Entity.h"
#include "core/World.h"
#include "core/Universe.h"

#include <algorithm>
#include <iterator>

shared_ptr<Entity> mapToOwner(shared_ptr<Component> component)
{
    return component ? component->getOwner() : nullptr;
}
