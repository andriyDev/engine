
cmake_minimum_required(VERSION 3.0)

project(Engine VERSION 0.1)

find_package(glm REQUIRED)

set(DEBUG_SHOW_CONSOLE ON CACHE BOOL "Specifies whether the console should be shown in debug mode.")

# Hide console window.
if(MSVC AND NOT DEBUG_SHOW_CONSOLE)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif()
if(MSVC)
	set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif()

add_subdirectory(src)
add_subdirectory(tools)
