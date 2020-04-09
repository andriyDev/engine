
cmake_minimum_required(VERSION 3.00)

project(Engine VERSION 0.1)

find_package(OpenGL REQUIRED)
find_package(GLM REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLFW3 REQUIRED)

set(OPENGL_INCLUDE "")
set(OPENGL_LIB ${OPENGL_gl_LIBRARY} ${OPENGL_glu_LIBRARY})

set(GLM_INCLUDE ${GLM_INCLUDE_DIR})
set(GLM_LIB "")

set(GLEW_INCLUDE ${GLEW_INCLUDE_DIR})
set(GLEW_LIB ${GLEW_STATIC_LIBRARY_RELEASE})

set(GLFW_INCLUDE "")
set(GLFW_LIB glfw)


set(MAIN_INCLUDE ${OPENGL_INCLUDE} ${GLM_INCLUDE} ${GLEW_INCLUDE} ${GLFW_INCLUDE})
set(MAIN_LIB ${OPENGL_LIB} ${GLM_LIB} ${GLEW_LIB} ${GLFW_LIB})

set(MAIN_SRC src/main.cpp)
set(MAIN_SRC ${MAIN_SRC}
    src/core/Query.cpp
    src/core/Universe.cpp
    src/core/Entity.cpp
    src/core/World.cpp)
set(MAIN_SRC ${MAIN_SRC}
    src/components/Transform.cpp)

add_executable(engine ${MAIN_SRC})

target_include_directories(engine PRIVATE ${MAIN_INCLUDE} include/)
target_link_libraries(engine ${MAIN_LIB})

