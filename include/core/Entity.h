
#pragma once

#include "std.h"

class Entity
{
public:
    inline uint getId() const {
        return id;
    }
    inline set<uint> getComponentIds() const {
        return components;
    }
private:
    uint id = 0;
    set<uint> components;

    friend class Universe;
};
