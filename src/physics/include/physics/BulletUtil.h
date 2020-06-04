
#pragma once

#include <bullet/btBulletCollisionCommon.h>
#include <glm/glm.hpp>
#include "components/Transform.h"

inline btVector3 convert(const vec3& v) {
    return btVector3(btScalar(v.x), btScalar(v.y), btScalar(v.z));
}
inline vec3 convert(const btVector3& v) {
    return vec3(v.getX(), v.getY(), v.getZ());
}
inline btQuaternion convert(const quat& q) {
    return btQuaternion(btScalar(q.x), btScalar(q.y), btScalar(q.z), btScalar(q.w));
}
inline quat convert(const btQuaternion& q) {
    return quat(q.getW(), q.getX(), q.getY(), q.getZ());
}
inline btTransform convert(const TransformData& t) {
    return btTransform(convert(t.rotation), convert(t.translation));
}
inline TransformData convert(const btTransform& t) {
    return TransformData(convert(t.getOrigin()), convert(t.getRotation()));
}