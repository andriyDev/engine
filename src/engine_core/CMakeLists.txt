
set(SRC)
list(APPEND SRC src/Entity.cpp)
list(APPEND SRC src/Query.cpp)
list(APPEND SRC src/Universe.cpp)
list(APPEND SRC src/World.cpp)
list(APPEND SRC src/Transform.cpp)

list(TRANSFORM SRC PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/)

add_library(engine_core ${SRC})

target_include_directories(engine_core
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE src)

target_link_libraries(engine_core glm::glm)
