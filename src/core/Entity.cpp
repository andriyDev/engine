
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

Component* Entity::findComponentByType(int typeId)
{
    for(Component* component : components) {
        if(component->getTypeId() == typeId) {
            return component;
        }
    }
    return nullptr;
}

std::set<Component*> Entity::findComponentsByType(int typeId)
{
    std::set<Component*> typedComponents;
    for(Component* component : components) {
        if(component->getTypeId() == typeId) {
            typedComponents.insert(component);
        }
    }
    return typedComponents;
}
