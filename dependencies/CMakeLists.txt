# ============================================
# Dependencies/CMakeLists.txt
# ============================================

# --------------------------------------------
# GLAD (built from source)
# --------------------------------------------

add_library(Glad STATIC source/glad.c)

target_include_directories(Glad PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set_target_properties(Glad PROPERTIES
    FOLDER "Dependencies"
)

# --------------------------------------------
# GLFW (prebuilt imported lib)
# --------------------------------------------

add_library(GLFW STATIC IMPORTED GLOBAL)

set_target_properties(GLFW PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/lib/glfw3.lib"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include"
    FOLDER "Dependencies"
)

# --------------------------------------------
# Dear ImGui (built from source)
# --------------------------------------------

set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/External/imgui)

file(GLOB_RECURSE IMGUI_SOURCES
    "${IMGUI_DIR}/*.cpp"
)

add_library(ImGui STATIC
    ${IMGUI_SOURCES}
)
target_include_directories(ImGui PUBLIC
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

target_link_libraries(ImGui PUBLIC
    GLFW
    Glad
)

set_target_properties(ImGui PROPERTIES
    FOLDER "Dependencies"
)

# --------------------------------------------
# irrKlang (imported)
# --------------------------------------------

add_library(IrrKlang STATIC IMPORTED GLOBAL)

set_target_properties(IrrKlang PROPERTIES
    IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/External/irrKlang/lib/irrKlang.lib"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/External/irrKlang/include"
    FOLDER "Dependencies"
)

# --------------------------------------------
# FBX SDK (imported)
# --------------------------------------------

set(FBX_ROOT "${CMAKE_SOURCE_DIR}/External/FBX_SDK")

add_library(FBX STATIC IMPORTED GLOBAL)

set_target_properties(FBX PROPERTIES
    IMPORTED_LOCATION_DEBUG   "${FBX_ROOT}/lib/x64/debug/libfbxsdk.lib"
    IMPORTED_LOCATION_RELEASE "${FBX_ROOT}/lib/x64/release/libfbxsdk.lib"
    MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release
    MAP_IMPORTED_CONFIG_MINSIZEREL     Release
    INTERFACE_INCLUDE_DIRECTORIES "${FBX_ROOT}/include"
    FOLDER "Dependencies"
)

# --------------------------------------------
# PhysX (interface target)
# --------------------------------------------

set(PHYSX_ROOT "${CMAKE_SOURCE_DIR}/External/PhysX")

add_library(PhysX INTERFACE)

target_include_directories(PhysX INTERFACE
    ${PHYSX_ROOT}/include
    ${PHYSX_ROOT}/pxshared/include
)

target_compile_definitions(PhysX INTERFACE
    PX_PHYSX_STATIC_LIB
)

target_link_libraries(PhysX INTERFACE
    # Debug
    $<$<CONFIG:Debug>:${PHYSX_ROOT}/lib/Debug/PhysX_64.lib>
    $<$<CONFIG:Debug>:${PHYSX_ROOT}/lib/Debug/PhysXCommon_64.lib>
    $<$<CONFIG:Debug>:${PHYSX_ROOT}/lib/Debug/PhysXFoundation_64.lib>
    $<$<CONFIG:Debug>:${PHYSX_ROOT}/lib/Debug/PhysXCooking_64.lib>
    $<$<CONFIG:Debug>:${PHYSX_ROOT}/lib/Debug/PhysXExtensions_static_64.lib>

    # Release
    $<$<CONFIG:Release>:${PHYSX_ROOT}/lib/Release/PhysX_64.lib>
    $<$<CONFIG:Release>:${PHYSX_ROOT}/lib/Release/PhysXCommon_64.lib>
    $<$<CONFIG:Release>:${PHYSX_ROOT}/lib/Release/PhysXFoundation_64.lib>
    $<$<CONFIG:Release>:${PHYSX_ROOT}/lib/Release/PhysXCooking_64.lib>
    $<$<CONFIG:Release>:${PHYSX_ROOT}/lib/Release/PhysXExtensions_static_64.lib>
)


# --------------------------------------------
# Luajit (imported)
# --------------------------------------------

set(LUA_ROOT "${CMAKE_SOURCE_DIR}/External/Lua")

add_library(LUA STATIC IMPORTED GLOBAL)

set_target_properties(LUA PROPERTIES
    IMPORTED_LOCATION_DEBUG   "${LUA_ROOT}/lib/Debug/lua51.lib"
    IMPORTED_LOCATION_RELEASE "${LUA_ROOT}/lib/Release/lua51.lib"
    MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release
    MAP_IMPORTED_CONFIG_MINSIZEREL     Release
    INTERFACE_INCLUDE_DIRECTORIES "${LUA_ROOT}/include"
)