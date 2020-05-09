
#pragma once

#include "std.h"

class Component
{
protected:
    Component(uint typeId) {
        this->typeId = typeId;
    }
public:
    inline uint getTypeId() const {
        return typeId;
    }
    inline std::shared_ptr<Entity> getOwner() const {
        return owner.lock();
    }
private:
    uint typeId; // The type id that determines this component.
    std::weak_ptr<Entity> owner; // The entity this component is owned by.

    friend class Universe;
    friend class Entity;
};
