
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
    
    /*
    Removes the component from this entities ownership.
    */
    void removeComponent(std::shared_ptr<Component> component);
    // Finds any component satisfying the type and removes it.
    std::shared_ptr<Component> removeComponentByType(uint typeId);
    // Finds all components satisfying the type and removes them.
    void removeComponentsByType(uint typeId);
    template<typename T>
    std::shared_ptr<T> removeComponent() {
        return std::static_pointer_cast<T>(removeComponentByType(get_id(T)));
    }
    template<typename T>
    void removeComponents() {
        removeComponentsByType(get_id(T));
    }

    // Finds any component attached to this entity with the specified type.
    std::shared_ptr<Component> findComponentByType(uint typeId);
    // Finds all components attached to this entity with the specified type.
    std::set<std::shared_ptr<Component>> findComponentsByType(uint typeId);

    template<typename T>
    std::shared_ptr<T> findComponent() {
        return std::static_pointer_cast<T>(findComponentByType(get_id(T)));
    }

    template<typename T>
    std::set<std::shared_ptr<T>> findComponents() {
        std::set<std::shared_ptr<Component>> components = findComponentsByType(get_id(T));
        std::set<std::shared_ptr<T>> results;
        for(std::shared_ptr<Component>& c : components) {
            results.insert(std::static_pointer_cast<T>(c));
        }
        return results;
    }

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
