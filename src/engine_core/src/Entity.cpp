
#include "core/Entity.h"
#include "core/Component.h"
#include "core/World.h"

Query<shared_ptr<Component>> Entity::queryComponents()
{
    return Query<shared_ptr<Component>>(components);
}

void Entity::addComponent(shared_ptr<Component> component)
{
    components.insert(component);
    component->owner = shared_from_this();
    shared_ptr<World> worldPtr = world.lock();
    if(worldPtr) {
        worldPtr->addComponent(component);
    }
}

void Entity::removeComponent(shared_ptr<Component> component)
{
    if(!component) {
        return;
    }
    auto it = components.find(component);
    if(it != components.end()) {
        components.erase(it);
    }
    shared_ptr<World> worldPtr = world.lock();
    if(worldPtr) {
        worldPtr->removeComponent(component);
    }
}

shared_ptr<Component> Entity::removeComponentByType(uint typeId)
{
    shared_ptr<World> worldPtr = world.lock();
    for(auto it = components.begin(); it != components.end(); ++it) {
        if((*it)->getTypeId() == typeId) {
            shared_ptr<Component> out = *it;
            components.erase(it);
            if(worldPtr) {
                worldPtr->removeComponent(out);
            }
            return out;
        }
    }
    return nullptr;
}

void Entity::removeComponentsByType(uint typeId)
{
    shared_ptr<World> worldPtr = world.lock();
    hash_set<shared_ptr<Component>> newComponents;
    for(auto it = components.begin(); it != components.end(); ++it) {
        if((*it)->getTypeId() != typeId) {
            newComponents.insert(*it);
        } else {
            if(worldPtr) {
                worldPtr->removeComponent(*it);
            }
        }
    }
    components = newComponents;
}

shared_ptr<Component> Entity::findComponentByType(uint typeId)
{
    for(shared_ptr<Component> component : components) {
        if(component->getTypeId() == typeId) {
            return component;
        }
    }
    return nullptr;
}

hash_set<shared_ptr<Component>> Entity::findComponentsByType(uint typeId)
{
    hash_set<shared_ptr<Component>> typedComponents;
    for(shared_ptr<Component> component : components) {
        if(component->getTypeId() == typeId) {
            typedComponents.insert(component);
        }
    }
    return typedComponents;
}
