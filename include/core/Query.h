
#pragma once

#include <functional>

#include "std.h"

class Entity;
class Component;
class World;

template<typename T>
class Query
{
public:
    // Constructs a query initially containing all items T in the provided world.
    Query<T>(const std::set<T>& _items) : items(_items) {}

    Query<T>& filter(std::function<bool(T)> predicate) {
        filters.push_back(predicate);
        return *this;
    }

    Query<T>& apply() {
        std::set<T> result;
        for(T t : *this) {
            bool pass = true;
            for(std::function<bool(T)> predicate : filters) {
                if(!predicate(t)) {
                    pass = false;
                    break;
                }
            }
            if(pass) { result.insert(t); }
        }
        filters.clear();
        items = std::move(result);
        return *this;
    }

    template<typename U>
    Query<U> map(std::function<U(T)> fcn) {
        if(!filters.empty()) {
            apply();
        }
        Query<U> result;
        for(T t : *this) {
            result.items.insert(fcn(t));
        }
        return result;
    }

    template<typename U>
    Query<U> cast() {
        if(!filters.empty()) {
            apply();
        }
        return map<U>([](T t){ return static_cast<U>(t); });
    }

    template<typename U>
    Query<std::shared_ptr<U>> cast_ptr() {
        if(!filters.empty()) {
            apply();
        }
        return map<std::shared_ptr<U>>([](T t){ return std::static_pointer_cast<U>(t); });
    }

    // Takes the union of the two queries (mutating the left Query).
    Query<T>& operator|=(const Query<T>& other) {
        if(!filters.empty()) {
            apply();
        }
        items.insert(other.begin(), other.end());
        return *this;
    }
    // Takes the union of the two queries (mutating the left Query).
    friend Query<T> operator|(Query<T> left, const Query<T>& right) {
        return left |= right;
    }
    // Takes the intersection of the two queries (mutating the left Query).
    Query<T>& operator&=(const Query<T>& other) {
        if(!filters.empty()) {
            apply();
        }
        set<T> result;

        std::set_intersection(
            items.begin(), items.end(),
            other.items.begin(), other.items.end(),
            std::inserter(result, result.end())
        );
        items = std::move(result);
        return *this;
    }
    // Takes the intersection of the two queries (mutating the left Query).
    friend Query<T> operator&(Query<T> left, const Query<T>& right) {
        return left &= right;
    }
    // Takes the difference of the two queries (mutating the left Query).
    Query<T>& operator-=(const Query<T>& other) {
        if(!filters.empty()) {
            apply();
        }
        std::set<T> result;

        std::set_difference(
            items.begin(), items.end(),
            other.items.begin(), other.items.end(),
            std::inserter(result, result.end())
        );
        items = std::move(result);
        return *this;
    }
    // Takes the difference of the two queries (mutating the left Query).
    friend Query<T> operator-(Query<T> left, const Query<T>& right) {
        return left -= right;
    }

    typename std::set<T>::iterator begin() const {
        return items.begin();
    }
    typename std::set<T>::iterator end() const {
        return items.end();
    }
private:
    std::set<T> items; // The set of items currently in the query
    std::vector<std::function<bool(T)>> filters; // The filters that are queued to be applied to the vector.

    Query<T>() {}

    friend class Query;
};

template<typename T>
bool filterByType(std::shared_ptr<Component> component) {
    return component->getTypeId() == get_id(T);
}

std::shared_ptr<Entity> mapToOwner(std::shared_ptr<Component> component);

template<typename T>
std::shared_ptr<T> mapToComponent(std::shared_ptr<Entity> entity)
{
    return entity ? entity->findComponent<T>() : nullptr;
}

template<typename T>
std::shared_ptr<T> mapToSibling(std::shared_ptr<Component> component)
{
    std::shared_ptr<Entity> owner = component->getOwner();
    return owner ? owner->findComponent<T>() : nullptr;
}
