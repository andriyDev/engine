
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
    Query<T>(const std::unordered_set<T>& _items) : items(_items) {}

    Query<T>& filter(std::function<bool(T)> predicate) {
        filters.push_back(predicate);
        return *this;
    }

    Query<T>& apply() {
        std::unordered_set<T> result;
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
    Query<std::shared_ptr<U>> map_ptr(std::function<std::shared_ptr<U> (T)> fcn) {
        if(!filters.empty()) {
            apply();
        }
        Query<std::shared_ptr<U>> result;
        for(T t : *this) {
            std::shared_ptr<U> u = fcn(t);
            if(u) {
                result.items.insert(u);
            }
        }
        return result;
    }

    template<typename U>
    Query<U> map_group(std::function<std::vector<U> (T)> fcn)
    {
        if(!filters.empty()) {
            apply();
        }
        Query<U> result;
        for(T t : *this) {
            std::vector<U> u = fcn(t);
            result.items.insert(u.begin(), u.end());
        }
        return result;
    }

    template<typename U>
    Query<std::shared_ptr<U>> map_group_ptr(std::function<std::vector<std::shared_ptr<U>> (T)> fcn)
    {
        if(!filters.empty()) {
            apply();
        }
        Query<std::shared_ptr<U>> result;
        for(T t : *this) {
            std::vector<std::shared_ptr<U>> u = fcn(t);
            result.items.insert(u.begin(), u.end());
        }
        return result;
    }

    template<typename U>
    Query<U> cast() {
        return map<U>([](T t){ return static_cast<U>(t); });
    }

    template<typename U>
    Query<std::shared_ptr<U>> cast_ptr() {
        return map<std::shared_ptr<U>>([](T t){ return std::static_pointer_cast<U>(t); });
    }

    bool contains(T t) const {
        return items.find(t) != items.end();
    }

    size_t size() const {
        return items.size();
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
        std::unordered_set<T> result;

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
        std::unordered_set<T> result;

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

    typename std::unordered_set<T>::iterator begin() const {
        return items.begin();
    }
    typename std::unordered_set<T>::iterator end() const {
        return items.end();
    }
private:
    std::unordered_set<T> items; // The set of items currently in the query
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
