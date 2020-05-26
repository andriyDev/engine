
cmake_minimum_required(VERSION 3.00)

project(Engine VERSION 0.1)

find_package(OpenGL REQUIRED)
find_package(GLM REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLFW3 REQUIRED)
find_package(Bullet REQUIRED)

set(OPENGL_INCLUDE "")
set(OPENGL_LIB ${OPENGL_gl_LIBRARY} ${OPENGL_glu_LIBRARY})

set(GLM_INCLUDE ${GLM_INCLUDE_DIR})
set(GLM_LIB "")

set(GLEW_INCLUDE ${GLEW_INCLUDE_DIR})
set(GLEW_LIB ${GLEW_STATIC_LIBRARY_RELEASE})

set(GLFW_INCLUDE "")
set(GLFW_LIB glfw)

set(BULLET_INCLUDE ${BULLET_INCLUDE_DIR})
set(BULLET_LIB_RELEASE ${BULLET_COLLISION_LIBRARY} ${BULLET_DYNAMICS_LIBRARY} ${BULLET_MATH_LIBRARY})
set(BULLET_LIB_DEBUG ${BULLET_COLLISION_LIBRARY_DEBUG} ${BULLET_DYNAMICS_LIBRARY_DEBUG} ${BULLET_MATH_LIBRARY_DEBUG})


set(MAIN_INCLUDE ${OPENGL_INCLUDE} ${GLM_INCLUDE} ${GLEW_INCLUDE} ${GLFW_INCLUDE} ${BULLET_INCLUDE})
set(MAIN_LIB ${OPENGL_LIB} ${GLM_LIB} ${GLEW_LIB} ${GLFW_LIB})
set(MAIN_LIB_RELEASE ${BULLET_LIB_RELEASE})
set(MAIN_LIB_DEBUG ${BULLET_LIB_DEBUG})

set(MAIN_SRC src/main.cpp
    src/Window.cpp
    src/InputSystem.cpp)
set(MAIN_SRC ${MAIN_SRC}
    src/core/Query.cpp
    src/core/Universe.cpp
    src/core/Entity.cpp
    src/core/World.cpp)
set(MAIN_SRC ${MAIN_SRC}
    src/components/Transform.cpp
    src/components/MeshRenderer.cpp
    src/components/Camera.cpp)
set(MAIN_SRC ${MAIN_SRC}
    src/physics/StaticBody.cpp
    src/physics/CollisionObject.cpp
    src/physics/Collider.cpp
    src/physics/RigidBody.cpp
    src/physics/KinematicBody.cpp
    src/physics/BoxCollider.cpp
    src/physics/SphereCollider.cpp
    src/physics/PhysicsSystem.cpp)
set(MAIN_SRC ${MAIN_SRC}
    src/resources/FileResourceBuilder.cpp
    src/resources/ResourceLoader.cpp
    src/resources/Mesh.cpp
    src/resources/Shader.cpp
    src/resources/Colour.cpp
    src/resources/Texture.cpp)
set(MAIN_SRC ${MAIN_SRC}
    src/utility/Package.cpp
    src/utility/Serializer.cpp)
set(MAIN_SRC ${MAIN_SRC}
    src/renderer/RenderSystem.cpp
    src/renderer/Material.cpp
    src/renderer/MaterialProgram.cpp
    src/renderer/RenderableTexture.cpp
    src/renderer/RenderableMesh.cpp)

add_executable(engine ${MAIN_SRC})

target_include_directories(engine PRIVATE ${MAIN_INCLUDE} include/)
target_link_libraries(engine ${MAIN_LIB})
target_link_libraries(engine debug ${MAIN_LIB_DEBUG})
target_link_libraries(engine optimized ${MAIN_LIB_RELEASE})

if(WIN32)
target_link_libraries(engine Ws2_32.lib)
endif (WIN32)

set(DEBUG_SHOW_CONSOLE ON CACHE BOOL "Specifies whether the console should be shown in debug mode.")

# Hide console window.
if(MSVC AND NOT DEBUG_SHOW_CONSOLE)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif()
if(MSVC)
	set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif()

set(BUILD_TOOLS ON CACHE BOOL "Should tools be built? Requires several more dependencies")
if(BUILD_TOOLS)
    add_subdirectory(tools)
endif()
