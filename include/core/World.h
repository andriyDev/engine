
#pragma once

#include "std.h"

class Query;

class World
{
public:
    Query query();

    inline uint getId() const {
        return id;
    }
    inline set<uint> getEntities() const {
        return entities;
    }
private:
    uint id;
    set<uint> entities;

    friend class Universe;
};
