
set(SRC)
list(APPEND SRC src/BoxCollider.cpp)
list(APPEND SRC src/Collider.cpp)
list(APPEND SRC src/CollisionObject.cpp)
list(APPEND SRC src/ConvexCollider.cpp)
list(APPEND SRC src/ConvexHull.cpp)
list(APPEND SRC src/KinematicBody.cpp)
list(APPEND SRC src/PhysicsSystem.cpp)
list(APPEND SRC src/PhysicsSystemQueries.cpp)
list(APPEND SRC src/RigidBody.cpp)
list(APPEND SRC src/SphereCollider.cpp)
list(APPEND SRC src/StaticBody.cpp)
list(APPEND SRC src/Trigger.cpp)

list(TRANSFORM SRC PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/)

# Set up Bullet libraries.
find_package(Bullet REQUIRED)
add_library(LinearMath STATIC IMPORTED)
set_property(TARGET LinearMath PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${BULLET_INCLUDE_DIR})
set_property(TARGET LinearMath PROPERTY IMPORTED_LOCATION_DEBUG ${BULLET_MATH_LIBRARY_DEBUG})
set_property(TARGET LinearMath PROPERTY IMPORTED_LOCATION_RELEASE ${BULLET_MATH_LIBRARY})

add_library(BulletCollision STATIC IMPORTED)
set_property(TARGET BulletCollision PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${BULLET_INCLUDE_DIR})
set_property(TARGET BulletCollision PROPERTY IMPORTED_LOCATION_DEBUG ${BULLET_COLLISION_LIBRARY_DEBUG})
set_property(TARGET BulletCollision PROPERTY IMPORTED_LOCATION_RELEASE ${BULLET_COLLISION_LIBRARY})

add_library(BulletDynamics STATIC IMPORTED)
set_property(TARGET BulletDynamics PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${BULLET_INCLUDE_DIR})
set_property(TARGET BulletDynamics PROPERTY IMPORTED_LOCATION_DEBUG ${BULLET_DYNAMICS_LIBRARY_DEBUG})
set_property(TARGET BulletDynamics PROPERTY IMPORTED_LOCATION_RELEASE ${BULLET_DYNAMICS_LIBRARY})

add_library(physics ${SRC})

target_include_directories(physics
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE src ${BULLET_INCLUDE_DIRS})

target_link_libraries(physics
    PRIVATE LinearMath BulletCollision BulletDynamics
    PUBLIC engine_core resource_system base_resources)
