
#include "core/Entity.h"
#include "core/Component.h"

Entity* Entity::attach(Component* component)
{
    assert(!component->ownerId);
    component->ownerId = id;
    components.insert(component);
    return this;
}

Entity* Entity::detach(Component* component)
{
    assert(component->ownerId == id);
    component->ownerId = 0;
    components.erase(component);
    return this;
}
