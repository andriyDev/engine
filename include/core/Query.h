
#pragma once

#include "std.h"

class World;
class Universe;

typedef bool inclusivity;
#define INCLUSIVE false
#define EXCLUSIVE true

class Query
{
public:
    Query(World* world);

    /*
    Filters entities in this query by the given component type id.
    Specifically, if inverted is INCLUSIVE, the query will only contain entities with the component.
    if inverted is EXCLUSIVE, the query will only contain entities without the component.
    
    Returns a reference to this query, mutated.
    */
    Query& filter(uint componentTypeId, inclusivity inverted=INCLUSIVE);

    // Takes the union of the two queries (mutating the left Query).
    Query& operator|=(const Query& other);
    // Takes the union of the two queries (mutating the left Query).
    friend Query operator|(Query left, const Query& right) {
        return left |= right;
    }
    // Takes the intersection of the two queries (mutating the left Query).
    Query& operator&=(const Query& other);
    // Takes the intersection of the two queries (mutating the left Query).
    friend Query operator&(Query left, const Query& right) {
        return left &= right;
    }
    // Takes the difference of the two queries (mutating the left Query).
    Query& operator-=(const Query& other);
    // Takes the difference of the two queries (mutating the left Query).
    friend Query operator-(Query left, const Query& right) {
        return left -= right;
    }
    /*
    Inverts the query.
    Specifically, entities in the world not in this query will be in the resulting query.
    Returns a reference to this query, mutated.
    */
    Query& operator~();
private:
    set<uint> entityIds; // The set of entities currently in the query
    World* world; // The world which the query is taking place for.
    Universe* universe; // The universe for this query.
};
