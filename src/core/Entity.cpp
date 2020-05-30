
#include "core/Entity.h"
#include "core/Component.h"

void Entity::addComponent(std::shared_ptr<Component> component)
{
    components.insert(component);
    component->owner = shared_from_this();
}

std::shared_ptr<Component> Entity::findComponentByType(uint typeId)
{
    for(std::shared_ptr<Component> component : components) {
        if(component->getTypeId() == typeId) {
            return component;
        }
    }
    return nullptr;
}

std::set<std::shared_ptr<Component>> Entity::findComponentsByType(uint typeId)
{
    std::set<std::shared_ptr<Component>> typedComponents;
    for(std::shared_ptr<Component> component : components) {
        if(component->getTypeId() == typeId) {
            typedComponents.insert(component);
        }
    }
    return typedComponents;
}
