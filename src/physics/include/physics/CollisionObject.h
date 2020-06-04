
#pragma once

#include "std.h"
#include "components/Transform.h"
#include "Collider.h"

class CollisionObject : public Transformable
{
public:
    CollisionObject(uint typeId) : Transformable(typeId) {}

    virtual class btCollisionObject* constructObject(class btCollisionShape* shape, class btMotionState* motion) = 0;

    vector<weak_ptr<Collider>> colliders;

    vector<shared_ptr<Collider>> getColliders();

    btCollisionObject* getBody() const { return body; }
    
    struct Contact
    {
        vec3 worldPoint;
        vec3 localPoint;
        vec3 normal;

        Contact() {}
        Contact(const vec3& _worldPoint, const vec3& _localPoint, const vec3& _normal)
            : worldPoint(_worldPoint), localPoint(_localPoint), normal(_normal) {}
    };

    struct Hit
    {
        weak_ptr<CollisionObject> other;
        vector<Contact> contacts;
    };

    inline vector<Hit> getHits() const { return hits; }
    Hit* findHit(shared_ptr<CollisionObject> other);
private:
    Hit* findOrCreateHit(shared_ptr<CollisionObject> other);

    btCollisionObject* body = nullptr;

    vector<Hit> hits;

    friend class PhysicsSystem;
};
