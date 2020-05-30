
#pragma once

#include "std.h"
#include "components/Transform.h"
#include "Collider.h"

class CollisionObject : public Transformable
{
public:
    CollisionObject(uint typeId) : Transformable(typeId) {}

    virtual class btCollisionObject* constructObject(class btCollisionShape* shape, class btMotionState* motion) = 0;

    std::vector<std::weak_ptr<Collider>> colliders;

    std::vector<std::shared_ptr<Collider>> getColliders();

    btCollisionObject* getBody() const { return body; }
    
    struct Contact
    {
        glm::vec3 worldPoint;
        glm::vec3 localPoint;
        glm::vec3 normal;

        Contact() {}
        Contact(const glm::vec3& _worldPoint, const glm::vec3& _localPoint, const glm::vec3& _normal)
            : worldPoint(_worldPoint), localPoint(_localPoint), normal(_normal) {}
    };

    struct Hit
    {
        std::weak_ptr<CollisionObject> other;
        std::vector<Contact> contacts;
    };

    inline std::vector<Hit> getHits() const { return hits; }
    Hit* findHit(std::shared_ptr<CollisionObject> other);
private:
    Hit* findOrCreateHit(std::shared_ptr<CollisionObject> other);

    btCollisionObject* body = nullptr;

    std::vector<Hit> hits;

    friend class PhysicsSystem;
};
