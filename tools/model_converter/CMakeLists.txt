

find_package(zlib REQUIRED)
find_package(IrrXML REQUIRED)
find_package(ASSIMP REQUIRED)

set(ASSIMP_INCLUDE ${ASSIMP_INCLUDE_DIRS})
set(ASSIMP_LIB ${IRRXML_LIBRARY} ${ZLIB_LIBRARY} assimp::assimp)

set(INCLUDE "")
set(LIB ${ASSIMP_LIB})
set(SRC src/main.cpp
    src/Serializer.cpp
    src/Package.cpp)

add_executable(model_converter ${SRC})

target_include_directories(model_converter PRIVATE ${INCLUDE} ${CMAKE_SOURCE_DIR}/include/ ${CMAKE_CURRENT_SOURCE_DIR}/include/)
target_link_libraries(model_converter ${LIB})

if(WIN32)
target_link_libraries(model_converter Ws2_32.lib)
endif (WIN32)
