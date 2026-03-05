set(EDITOR_NAME Editor) #Local variable only

file(GLOB_RECURSE PROJECT_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Header/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp
)

add_library(${EDITOR_NAME})

target_sources(${EDITOR_NAME} PRIVATE ${PROJECT_FILES})

target_include_directories(${EDITOR_NAME} 
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Header
	PRIVATE ${RESOURCES_INCLUDE_DIR}
	PRIVATE ${LOW_INCLUDE_DIR}
	PRIVATE ${CORE_INCLUDE_DIR}
)

target_include_directories(${EDITOR_NAME} PRIVATE ${LIB_INCLUDE_DIR})
target_link_libraries(${EDITOR_NAME}
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
    PRIVATE ${CORE_NAME}
)
set(EDITOR_NAME ${EDITOR_NAME} PARENT_SCOPE) #outer scope variable only

set(EDITOR_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Header PARENT_SCOPE)
#This command sets the LIB_INCLUDE_DIR variable to the path of the 'include' directory
