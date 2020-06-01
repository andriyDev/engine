
set(SRC)
list(APPEND SRC src/FileResource.cpp)
list(APPEND SRC src/ResourceLoader.cpp)
list(APPEND SRC src/Serializer.cpp)

list(TRANSFORM SRC PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

add_library(resource_system ${SRC})

target_include_directories(resource_system
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE src)

if(WIN32)
set(HOST_NET_CONVERSION Ws2_32.lib)
else (WIN32)
set(HOST_NET_CONVERSION) # TODO: Fill this for *nix and Mac.
endif (WIN32)

target_link_libraries(resource_system
    PUBLIC engine_core
    PRIVATE ${HOST_NET_CONVERSION})
