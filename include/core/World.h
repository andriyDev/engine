
#pragma once

#include "std.h"

class World
{
public:
    Query query();

    inline set<uint> getEntities() const {
        return entities;
    }
private:
    set<uint> entities;
};
