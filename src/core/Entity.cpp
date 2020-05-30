
#include "core/Entity.h"
#include "core/Component.h"

Query<std::shared_ptr<Component>> Entity::queryComponents()
{
    return Query<std::shared_ptr<Component>>(components);
}

void Entity::addComponent(std::shared_ptr<Component> component)
{
    components.insert(component);
    component->owner = shared_from_this();
}

void Entity::removeComponent(std::shared_ptr<Component> component)
{
    if(!component) {
        return;
    }
    auto it = components.find(component);
    if(it != components.end()) {
        components.erase(it);
    }
}

std::shared_ptr<Component> Entity::removeComponentByType(uint typeId)
{
    for(auto it = components.begin(); it != components.end(); ++it) {
        if((*it)->getTypeId() == typeId) {
            std::shared_ptr<Component> out = *it;
            components.erase(it);
            return out;
        }
    }
    return nullptr;
}

void Entity::removeComponentsByType(uint typeId)
{
    std::set<std::shared_ptr<Component>> newComponents;
    for(auto it = components.begin(); it != components.end(); ++it) {
        if((*it)->getTypeId() != typeId) {
            newComponents.insert(*it);
        }
    }
    components = newComponents;
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
