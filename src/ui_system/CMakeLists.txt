
set(SRC)
list(APPEND SRC src/AnchorOffsetLayout.cpp)
list(APPEND SRC src/Box.cpp)
list(APPEND SRC src/UISystem.cpp)
list(APPEND SRC src/UIUtil.cpp)

list(TRANSFORM SRC PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

add_library(ui_system ${SRC})

target_include_directories(ui_system
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE src)

target_link_libraries(ui_system
    PUBLIC engine_core
    PRIVATE renderer resource_system base_resources)
