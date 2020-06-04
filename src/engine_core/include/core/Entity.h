
#pragma once

#include "std.h"
#include "core/Query.h"

class World;
class Component;

class Entity : public enable_shared_from_this<Entity>
{
public:
    // Returns a query that can filter down components owned by the entity.
    Query<shared_ptr<Component>> queryComponents();

    /*
    Transfers ownership of the component to this entity.
    Do not store shared_ptrs to the component afterwards.
    */
    void addComponent(shared_ptr<Component> component);
    template<typename T>
    shared_ptr<T> addComponent()
    {
        shared_ptr<T> t = make_shared<T>();
        addComponent(t);
        return t;
    }
    
    /*
    Removes the component from this entities ownership.
    */
    void removeComponent(shared_ptr<Component> component);
    // Finds any component satisfying the type and removes it.
    shared_ptr<Component> removeComponentByType(uint typeId);
    // Finds all components satisfying the type and removes them.
    void removeComponentsByType(uint typeId);
    template<typename T>
    shared_ptr<T> removeComponent() {
        return static_pointer_cast<T>(removeComponentByType(get_id(T)));
    }
    template<typename T>
    void removeComponents() {
        removeComponentsByType(get_id(T));
    }

    // Finds any component attached to this entity with the specified type.
    shared_ptr<Component> findComponentByType(uint typeId);
    // Finds all components attached to this entity with the specified type.
    hash_set<shared_ptr<Component>> findComponentsByType(uint typeId);

    template<typename T>
    shared_ptr<T> findComponent() {
        return static_pointer_cast<T>(findComponentByType(get_id(T)));
    }

    template<typename T>
    hash_set<shared_ptr<T>> findComponents() {
        hash_set<shared_ptr<Component>> components = findComponentsByType(get_id(T));
        hash_set<shared_ptr<T>> results;
        for(shared_ptr<Component>& c : components) {
            results.insert(static_pointer_cast<T>(c));
        }
        return results;
    }

    inline shared_ptr<World> getWorld() const {
        return world.lock();
    }
    inline hash_set<shared_ptr<Component>> getComponents() const {
        return components;
    }
private:
    weak_ptr<World> world; // The world this entity is in.
    hash_set<shared_ptr<Component>> components; // The components attached to this entity.

    friend class Universe;
    friend class World;
};
