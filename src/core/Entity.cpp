
#include "core/Entity.h"
#include "core/Component.h"

Entity* Entity::attach(Component* component)
{
    assert(!component->ownerId);
    component->ownerId = id;
    components.insert(component->id);
    return this;
}
