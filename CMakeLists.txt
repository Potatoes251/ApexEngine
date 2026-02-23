cmake_minimum_required(VERSION 3.24 FATAL_ERROR) # for PATH_EQUAL

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (MSVC)
    add_compile_options(/W4)
    add_compile_options(/wd4505)
    add_compile_options(/WX)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL$<$<CONFIG:Debug>:Debug>")
endif()

set(EXE_NAME Engine)
project(${EXE_NAME} LANGUAGES C CXX)

# Collect assets
file(GLOB_RECURSE ASSET_FILES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/assets/*.glsl
    ${CMAKE_SOURCE_DIR}/assets/*.obj
    ${CMAKE_SOURCE_DIR}/assets/*.png
    ${CMAKE_SOURCE_DIR}/assets/*.jpg
    ${CMAKE_SOURCE_DIR}/assets/*.tga
)

# Group them properly in Visual Studio
source_group(TREE ${CMAKE_SOURCE_DIR}/assets
             PREFIX "Assets"
             FILES ${ASSET_FILES})

add_custom_target(Assets SOURCES
    ${ASSET_FILES}
)

add_subdirectory(assets)
add_subdirectory(dependencies)
add_subdirectory(LibMath)
add_subdirectory(Resources)
add_subdirectory(LowRenderer)
add_subdirectory(Core)


if (MSVC)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Engine)
endif()