
#include "core/Query.h"
#include "core/Component.h"
#include "core/Entity.h"
#include "core/World.h"
#include "core/Universe.h"

#include <algorithm>
#include <iterator>

std::function<bool(std::shared_ptr<Component>)> filterByTypeId(uint typeId)
{
    return [typeId](std::shared_ptr<Component> C) { return C->getTypeId() == typeId; };
}

std::shared_ptr<Entity> mapToOwner(std::shared_ptr<Component> component)
{
    return component->getOwner();
}
