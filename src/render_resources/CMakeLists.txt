
set(SRC)
list(APPEND SRC src/Material.cpp)
list(APPEND SRC src/MaterialProgram.cpp)
list(APPEND SRC src/RenderableMesh.cpp)
list(APPEND SRC src/RenderableTexture.cpp)

list(TRANSFORM SRC PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/)

find_package(GLEW REQUIRED)

add_library(glew STATIC IMPORTED)
set_property(TARGET glew PROPERTY INCLUDE_DIRECTORIES ${GLEW_INCLUDE_DIR})
set_property(TARGET glew PROPERTY IMPORTED_LOCATION_DEBUG ${GLEW_STATIC_LIBRARY_RELEASE})
set_property(TARGET glew PROPERTY IMPORTED_LOCATION_RELEASE ${GLEW_STATIC_LIBRARY_RELEASE})

find_package(OpenGL REQUIRED)

add_library(render_resources ${SRC})

target_include_directories(render_resources
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE src)

target_link_libraries(render_resources
    PUBLIC engine_core resource_system base_resources
    PRIVATE glew OpenGL::GL OpenGL::GLU)
