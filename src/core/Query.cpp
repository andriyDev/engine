
#include "core/Query.h"
#include "core/Component.h"
#include "core/Entity.h"
#include "core/World.h"
#include "core/Universe.h"

#include <algorithm>
#include <iterator>

std::shared_ptr<Entity> mapToOwner(std::shared_ptr<Component> component)
{
    return component ? component->getOwner() : nullptr;
}
