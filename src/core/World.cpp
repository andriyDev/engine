
#include "core/World.h"
#include "core/Query.h"

Query World::query()
{
    return Query(this);
}
