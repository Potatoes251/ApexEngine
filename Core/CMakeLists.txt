# -----------------------------------
# Core/CMakeLists.txt
# -----------------------------------

set(CORE_NAME Engine) #Local variable only

# Collect code files
file(GLOB_RECURSE PROJECT_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Header/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp
)

add_library(${CORE_NAME})

# Create Visual Studio filters matching folder structure
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${PROJECT_FILES})

target_sources(${CORE_NAME} PRIVATE ${PROJECT_FILES})

# ---- Include directories ----
target_include_directories(${CORE_NAME}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/header
    PRIVATE ${LIB_INCLUDE_DIR}
    PRIVATE ${LOW_INCLUDE_DIR}
    PRIVATE ${RESOURCES_INCLUDE_DIR}
    PRIVATE ${UI_INCLUDE_DIR}
    PRIVATE ${PHYS_INCLUDE_DIR}
)

# ---- Link libraries ----
target_link_libraries(${CORE_NAME}
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
    PRIVATE ${DAT_NAME}
    PRIVATE ${LOW_NAME}
    PRIVATE ${UI_NAME}
    PRIVATE ${PHYS_NAME}
)

set(CORE_NAME ${CORE_NAME} PARENT_SCOPE) #outer scope variable only

set(CORE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Header PARENT_SCOPE)