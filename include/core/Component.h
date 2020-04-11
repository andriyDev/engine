
#pragma once

#include "std.h"

class Component
{
protected:
    Component(uint typeId) {
        this->typeId = typeId;
    }
public:
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
    uint id = 0; // The id of the component.
    uint typeId; // The type id that determines this component.
    uint ownerId = 0; // The id of the entity that owns this component.

    friend class Universe;
    friend class Entity;
};
