
#include "physics/CollisionObject.h"

std::vector<std::shared_ptr<Collider>> CollisionObject::getColliders()
{
    std::vector<std::shared_ptr<Collider>> out;
    out.reserve(colliders.size());
    for(std::weak_ptr<Collider>& wptr : colliders) {
        std::shared_ptr<Collider> collider = wptr.lock();
        if(collider) {
            out.push_back(collider);
        }
    }
    return out;
}
