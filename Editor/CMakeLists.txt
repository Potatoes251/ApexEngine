# -----------------------------------
# Editor/CMakeLists.txt
# -----------------------------------

# Collect code files
file(GLOB_RECURSE PROJECT_HEADERS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Header/*.h)
file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp)

# Group files for IDE
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}
             FILES ${PROJECT_HEADERS} ${PROJECT_SOURCES})


# Create executable
add_executable(${EXE_EDITOR_NAME})

# Create Visual Studio filters matching folder structure
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${PROJECT_FILES})

# Add source files
target_sources(${EXE_EDITOR_NAME} PRIVATE ${PROJECT_HEADERS} ${PROJECT_SOURCES})

if (WIN32)
    # /ENTRY:mainCRTStartup keeps the same "main" function instead of requiring "WinMain"
    set(SUBSYSTEM_LINKER_OPTIONS "/SUBSYSTEM:WINDOWS" "/ENTRY:mainCRTStartup")
else()
    set(SUBSYSTEM_LINKER_OPTIONS "-mwindows")
endif()

target_link_options(${EXE_EDITOR_NAME} PRIVATE ${SUBSYSTEM_LINKER_OPTIONS})

# ---- Include directories ----
target_include_directories(${EXE_EDITOR_NAME}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/header
    PRIVATE ${LIB_INCLUDE_DIR}
    PRIVATE ${DAT_INCLUDE_DIR}
    PRIVATE ${LOW_INCLUDE_DIR}
    PRIVATE ${RESOURCES_INCLUDE_DIR}
    PRIVATE ${UI_INCLUDE_DIR}
    PRIVATE ${PHYS_INCLUDE_DIR}
    PRIVATE ${CORE_INCLUDE_DIR}
)

# ---- Link libraries ----
target_link_libraries(${EXE_EDITOR_NAME}
    PRIVATE Glad
    PRIVATE GLFW
    PRIVATE ImGui
    PRIVATE IrrKlang
    PRIVATE FBX
    PRIVATE PhysX
    PRIVATE LUA
    PRIVATE opengl32
    PRIVATE user32 
    PRIVATE gdi32 
    PRIVATE shell32
    PRIVATE ${LIB_NAME}
    PRIVATE ${RESOURCES_NAME}
    PRIVATE ${LOW_NAME}
    PRIVATE ${UI_NAME}
    PRIVATE ${PHYS_NAME}
    PRIVATE ${CORE_NAME}
)

add_dependencies(${EXE_EDITOR_NAME} ${EXE_NAME})

target_compile_definitions(${EXE_EDITOR_NAME} PRIVATE
    GAME_EXE_DIR="$<TARGET_FILE_DIR:${EXE_NAME}>"
)

add_custom_command(TARGET ${EXE_EDITOR_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:${EXE_EDITOR_NAME}>/assets
)

# This ensures the executable can find the DLL at runtime
add_custom_command(TARGET ${EXE_EDITOR_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/External/irrKlang/bin/irrKlang.dll"
        $<TARGET_FILE_DIR:${EXE_EDITOR_NAME}>
)

add_custom_command(TARGET ${EXE_EDITOR_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<$<CONFIG:Debug>:${CMAKE_SOURCE_DIR}/External/FBX_SDK/lib/x64/debug/libfbxsdk.dll>

    $<$<CONFIG:Release>:${CMAKE_SOURCE_DIR}/External/FBX_SDK/lib/x64/release/libfbxsdk.dll>

    $<TARGET_FILE_DIR:${EXE_EDITOR_NAME}>
)

set(PHYSX_ROOT "${CMAKE_SOURCE_DIR}/External/PhysX")

add_custom_command(TARGET ${EXE_EDITOR_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<$<CONFIG:Debug>:${PHYSX_ROOT}/bin/Debug/PhysX_64.dll>
        $<$<CONFIG:Debug>:${PHYSX_ROOT}/bin/Debug/PhysXCommon_64.dll>
        $<$<CONFIG:Debug>:${PHYSX_ROOT}/bin/Debug/PhysXFoundation_64.dll>
        $<$<CONFIG:Debug>:${PHYSX_ROOT}/bin/Debug/PhysXCooking_64.dll>

        $<$<CONFIG:Release>:${PHYSX_ROOT}/bin/Release/PhysX_64.dll>
        $<$<CONFIG:Release>:${PHYSX_ROOT}/bin/Release/PhysXCommon_64.dll>
        $<$<CONFIG:Release>:${PHYSX_ROOT}/bin/Release/PhysXFoundation_64.dll>
        $<$<CONFIG:Release>:${PHYSX_ROOT}/bin/Release/PhysXCooking_64.dll>
        $<TARGET_FILE_DIR:${EXE_EDITOR_NAME}>
)

add_custom_command(TARGET ${EXE_EDITOR_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<$<CONFIG:Debug>:${CMAKE_SOURCE_DIR}/External/Lua/bin/Debug/lua51.dll>

    $<$<CONFIG:Release>:${CMAKE_SOURCE_DIR}/External/Lua/bin/Release/lua51.dll>

    $<TARGET_FILE_DIR:${EXE_EDITOR_NAME}>
)
