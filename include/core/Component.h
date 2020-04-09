
#pragma once

#include "std.h"

class Component
{
public:
    Component(uint typeId) {
        this->typeId = typeId;
    }

    inline uint getId() const {
        return id;
    }
    inline uint getTypeId() const {
        return typeId;
    }
    inline uint getOwnerId() const {
        return ownerId;
    }
private:
    uint id = 0;
    uint typeId;
    uint ownerId = 0;

    friend class Universe;
};
