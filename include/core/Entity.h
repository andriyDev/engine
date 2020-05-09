
#pragma once

#include "std.h"

class Component;

class Entity : public std::enable_shared_from_this<Entity>
{
public:
    /*
    Transfers ownership of the component to this entity.
    Do not store shared_ptrs to the component afterwards.
    */
    void addComponent(std::shared_ptr<Component> component);

    // Finds any component attached to this entity with the specified type.
    std::shared_ptr<Component> findComponentByType(int typeId);
    // Finds all components attached to this entity with the specified type.
    std::set<std::shared_ptr<Component>> findComponentsByType(int typeId);

    inline std::shared_ptr<World> getWorld() const {
        return world.lock();
    }
    inline std::set<std::shared_ptr<Component>> getComponents() const {
        return components;
    }
private:
    std::weak_ptr<World> world; // The world this entity is in.
    std::set<std::shared_ptr<Component>> components; // The components attached to this entity.

    friend class Universe;
    friend class World;
};
