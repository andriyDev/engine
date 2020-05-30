
#pragma once

#include "std.h"

class World;
class Component;

class Entity : public std::enable_shared_from_this<Entity>
{
public:
    /*
    Transfers ownership of the component to this entity.
    Do not store shared_ptrs to the component afterwards.
    */
    void addComponent(std::shared_ptr<Component> component);
    template<typename T>
    std::shared_ptr<T> addComponent()
    {
        std::shared_ptr<T> t = std::make_shared<T>();
        addComponent(t);
        return t;
    }

    // Finds any component attached to this entity with the specified type.
    std::shared_ptr<Component> findComponentByType(uint typeId);
    // Finds all components attached to this entity with the specified type.
    std::set<std::shared_ptr<Component>> findComponentsByType(uint typeId);

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
