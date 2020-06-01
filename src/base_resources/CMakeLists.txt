
set(SRC)
list(APPEND SRC src/Colour.cpp)
list(APPEND SRC src/Mesh.cpp)
list(APPEND SRC src/Shader.cpp)
list(APPEND SRC src/Texture.cpp)

list(TRANSFORM SRC PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/)

add_library(base_resources ${SRC})

target_include_directories(base_resources
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE src)

target_link_libraries(base_resources engine_core resource_system)
