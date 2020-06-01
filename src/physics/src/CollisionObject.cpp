
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

CollisionObject::Hit* CollisionObject::findHit(std::shared_ptr<CollisionObject> other)
{
    for(Hit& hit : hits) {
        if(hit.other.lock() == other) {
            return &hit;
        }
    }
    return nullptr;
}

CollisionObject::Hit* CollisionObject::findOrCreateHit(std::shared_ptr<CollisionObject> other)
{
    Hit* hit = findHit(other);
    if(!hit) {
        Hit h;
        h.other = other;
        hits.push_back(h);
        hit = &hits.back();
    }
    return hit;
}
