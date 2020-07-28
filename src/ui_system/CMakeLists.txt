
set(SRC)
list(APPEND SRC src/Container.cpp)
list(APPEND SRC src/ListLayout.cpp)
list(APPEND SRC src/OverlayLayout.cpp)
list(APPEND SRC src/Box.cpp)
list(APPEND SRC src/Button.cpp)
list(APPEND SRC src/UIElement.cpp)
list(APPEND SRC src/WrapperElement.cpp)
list(APPEND SRC src/UISystem.cpp)
list(APPEND SRC src/UIUtil.cpp)
list(APPEND SRC src/Text.cpp)
list(APPEND SRC src/TextField.cpp)
list(APPEND SRC src/Image.cpp)
list(APPEND SRC src/Slider.cpp)

list(TRANSFORM SRC PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

add_library(ui_system ${SRC})

target_include_directories(ui_system
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE src)

target_link_libraries(ui_system
    PUBLIC engine_core font_resource
    PRIVATE renderer resource_system base_resources)
