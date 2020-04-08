
cmake_minimum_required(VERSION 3.00)

project(Engine VERSION 0.1)

find_package(OpenGL REQUIRED)
find_package(GLM REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLFW3 REQUIRED)

add_executable(engine src/main.cpp)
