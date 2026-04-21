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
    add_compile_options(/wd4100)
    ##add_compile_options(/wd4700)
    add_compile_options(/wd26495)
    add_compile_options(/wd33010)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

# --- Existing Assets ---
file(GLOB_RECURSE ASSET_FILES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/Assets/*
)

source_group(TREE ${CMAKE_SOURCE_DIR}/Assets
             PREFIX "Assets"
             FILES ${ASSET_FILES})

# --- Apex Assets ---
file(GLOB_RECURSE APEX_ASSET_FILES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/ApexAssets/*
)

source_group(TREE ${CMAKE_SOURCE_DIR}/ApexAssets
             PREFIX "ApexAssets"
             FILES ${APEX_ASSET_FILES})

add_custom_target(Assets SOURCES
    ${ASSET_FILES}
    ${APEX_ASSET_FILES}
)

add_subdirectory(Dependencies)
add_subdirectory(LibMath)
add_subdirectory(LowRenderer)
add_subdirectory(Resources)
add_subdirectory(UI)
add_subdirectory(Physic)
add_subdirectory(Core)
add_subdirectory(Game)
add_subdirectory(Editor)

if (MSVC)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${EXE_EDITOR_NAME})
endif()