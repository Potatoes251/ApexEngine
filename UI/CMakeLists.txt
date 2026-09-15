set(UI_NAME UI) #Local variable only

file(GLOB_RECURSE PROJECT_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Header/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp
)

add_library(${UI_NAME})

# Create Visual Studio filters matching folder structure
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${PROJECT_FILES})

target_sources(${UI_NAME} PRIVATE ${PROJECT_FILES})

target_include_directories(${UI_NAME} 
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Header
	PRIVATE ${RESOURCES_INCLUDE_DIR}
	PRIVATE ${LOW_INCLUDE_DIR}
)

target_include_directories(${UI_NAME} PRIVATE ${LIB_INCLUDE_DIR})
target_link_libraries(${UI_NAME}
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
)
set(UI_NAME ${UI_NAME} PARENT_SCOPE) #outer scope variable only

set(UI_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Header PARENT_SCOPE)
#This command sets the LIB_INCLUDE_DIR variable to the path of the 'include' directory
