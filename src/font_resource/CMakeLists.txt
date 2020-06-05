
set(SRC)
list(APPEND SRC src/Font.cpp)
list(APPEND SRC src/FontFace.cpp)

list(TRANSFORM SRC PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(Freetype REQUIRED)

add_library(font_resource ${SRC})

target_include_directories(font_resource
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE src)

target_link_libraries(font_resource
    PUBLIC engine_core resource_system render_resources
    PRIVATE base_resources Freetype::Freetype)
