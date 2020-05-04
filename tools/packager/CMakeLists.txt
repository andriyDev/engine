

find_package(zlib REQUIRED)
find_package(IrrXML REQUIRED)
find_package(ASSIMP REQUIRED)

set(ASSIMP_INCLUDE ${ASSIMP_INCLUDE_DIRS})
set(ASSIMP_LIB ${IRRXML_LIBRARY} ${ZLIB_LIBRARY} assimp::assimp)

set(INCLUDE "")
set(LIB ${ASSIMP_LIB})
set(SRC src/main.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/Mesh.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/Shader.cpp
    ${CMAKE_SOURCE_DIR}/src/utility/Serializer.cpp
    ${CMAKE_SOURCE_DIR}/src/utility/Package.cpp)

add_executable(packager ${SRC})

target_include_directories(packager PRIVATE ${INCLUDE} ${CMAKE_SOURCE_DIR}/include/ ${CMAKE_CURRENT_SOURCE_DIR}/include/)
target_link_libraries(packager ${LIB})

if(WIN32)
target_link_libraries(packager Ws2_32.lib)
endif (WIN32)