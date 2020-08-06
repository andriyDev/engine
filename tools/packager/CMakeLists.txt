
project(packager VERSION 0.1)

find_package(ZLIB REQUIRED)
find_package(IrrXML REQUIRED)
find_package(ASSIMP REQUIRED)
find_package(Freetype REQUIRED)
find_package(PNG REQUIRED)

set(SRC)
list(APPEND SRC src/main.cpp)
list(APPEND SRC src/font_load.cpp)

add_library(IrrXML STATIC IMPORTED)
set_property(TARGET IrrXML PROPERTY INCLUDE_DIRECTORIES ${IRRXML_INCLUDE_DIR})
set_property(TARGET IrrXML PROPERTY IMPORTED_LOCATION_DEBUG ${IRRXML_LIBRARY_DEBUG})
set_property(TARGET IrrXML PROPERTY IMPORTED_LOCATION_RELEASE ${IRRXML_LIBRARY})

add_executable(packager ${SRC})

target_include_directories(packager
    PRIVATE src)

target_link_libraries(packager
    PRIVATE resource_system base_resources font_resource assimp::assimp png_static IrrXML Freetype::Freetype)
