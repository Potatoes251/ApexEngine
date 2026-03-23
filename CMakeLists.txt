cmake_minimum_required(VERSION 3.24 FATAL_ERROR) # for PATH_EQUAL

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(EXE_NAME GameLauncher)
set(EXE_EDITOR_NAME Editor)
project(${EXE_NAME} LANGUAGES C CXX)

if (MSVC)
    add_compile_options(/W4)
    add_compile_options(/wd4505)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

# Collect assets
file(GLOB_RECURSE ASSET_FILES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/Assets/*
)

# Remove anything inside Assets/Private
list(FILTER ASSET_FILES EXCLUDE REGEX ".*/Assets/Internal/.*")

# Group them properly in Visual Studio
source_group(TREE ${CMAKE_SOURCE_DIR}/Assets
             PREFIX "Assets"
             FILES ${ASSET_FILES})

add_custom_target(Assets SOURCES
    ${ASSET_FILES}
)


add_subdirectory(Dependencies)
add_subdirectory(LibMath)
add_subdirectory(LowRenderer)
add_subdirectory(Resources)
add_subdirectory(UI)
add_subdirectory(Physic)
add_subdirectory(Core)
add_subdirectory(Editor)
add_subdirectory(Game)

if (MSVC)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${EXE_EDITOR_NAME})
endif()