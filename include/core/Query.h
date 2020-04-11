
#pragma once

#include <functional>

#include "std.h"

class Entity;
class Component;
class World;

typedef bool inclusivity;
#define INCLUSIVE false
#define EXCLUSIVE true

template<typename T>
class Query
{
public:
    // Constructs a query initially containing all items T in the provided world.
    Query<T>(World* world);

    Query<T>& filter(function<bool(T)> predicate) {
        set<T> result;
        for(T t : *this) {
            if(predicate(t)) {
                result.insert(t);
            }
        }
        items = move(result);
        return *this;
    }

    template<typename U>
    Query<U> map(function<U(T)> fcn) {
        Query<U> result;
        result.world = world;
        for(T t : *this) {
            result.items.insert(fcn(t));
        }
        return result;
    }

    template<typename U>
    Query<U> cast() {
        return map<U>([](T t){ return static_cast<U>(t); });
    }

    // Takes the union of the two queries (mutating the left Query).
    Query<T>& operator|=(const Query<T>& other) {
        items.insert(other.begin(), other.end());
        return *this;
    }
    // Takes the union of the two queries (mutating the left Query).
    friend Query<T> operator|(Query<T> left, const Query<T>& right) {
        return left |= right;
    }
    // Takes the intersection of the two queries (mutating the left Query).
    Query<T>& operator&=(const Query<T>& other) {
        set<T> result;

        set_intersection(
            items.begin(), items.end(),
            other.items.begin(), other.items.end(),
            inserter(result, result.end())
        );
        items = move(result);
        return *this;
    }
    // Takes the intersection of the two queries (mutating the left Query).
    friend Query<T> operator&(Query<T> left, const Query<T>& right) {
        return left &= right;
    }
    // Takes the difference of the two queries (mutating the left Query).
    Query<T>& operator-=(const Query<T>& other) {
        set<T> result;

        set_difference(
            items.begin(), items.end(),
            other.items.begin(), other.items.end(),
            inserter(result, result.end())
        );
        items = move(result);
        return *this;
    }
    // Takes the difference of the two queries (mutating the left Query).
    friend Query<T> operator-(Query<T> left, const Query<T>& right) {
        return left -= right;
    }
    /*
    Inverts the query.
    Specifically, entities in the world not in this query will be in the resulting query.
    Returns a reference to this query, mutated.
    */
    Query<T>& operator~() {
        set<T> result;
        Query<T> all(world);

        set_difference(
            all.items.begin(), all.items.end(),
            items.begin(), items.end(),
            inserter(result, result.end())
        );
        entities = move(result);
        return *this;
    }

    typename set<T>::iterator begin() const {
        return items.begin();
    }
    typename set<T>::iterator end() const {
        return items.end();
    }
private:
    set<T> items; // The set of items currently in the query
    World* world; // The world which the query is taking place for.

    Query<T>() {}

    friend class Query;
};

set<uint> toIdSet(const Query<Entity*>& query);
set<uint> toIdSet(const Query<Component*>& query);

function<bool(Component*)> filterByTypeId(uint typeId);
