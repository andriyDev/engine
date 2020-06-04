
set(DEBUG_SHOW_CONSOLE ON CACHE BOOL "Specifies whether the console should be shown in debug mode.")

# Hide console window.
if(MSVC AND NOT DEBUG_SHOW_CONSOLE)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif()
if(MSVC)
	set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif()

add_subdirectory(engine_core)
add_subdirectory(resource_system)
add_subdirectory(base_resources)
add_subdirectory(renderer)
add_subdirectory(physics)
add_subdirectory(ui_system)
add_subdirectory(main)
