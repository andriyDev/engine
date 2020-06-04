
project(engine_example VERSION 0.1)

set(SRC)
list(APPEND SRC src/debug.cpp)
list(APPEND SRC src/InputSystem.cpp)
list(APPEND SRC src/Window.cpp)
list(APPEND SRC src/main.cpp)

list(TRANSFORM SRC PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/)

find_package(GLFW3 REQUIRED)

find_package(GLEW REQUIRED)

add_library(glew STATIC IMPORTED)
set_property(TARGET glew PROPERTY INCLUDE_DIRECTORIES ${GLEW_INCLUDE_DIR})
set_property(TARGET glew PROPERTY IMPORTED_LOCATION_DEBUG ${GLEW_STATIC_LIBRARY_RELEASE})
set_property(TARGET glew PROPERTY IMPORTED_LOCATION_RELEASE ${GLEW_STATIC_LIBRARY_RELEASE})

add_executable(engine_example ${SRC})

target_include_directories(engine_example
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE src)

target_link_libraries(engine_example
    PRIVATE engine_core resource_system base_resources physics renderer glfw glew ui_system)

add_custom_command(TARGET engine_example POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/res $<TARGET_FILE_DIR:engine_example>)
