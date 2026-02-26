set(LOW_NAME LowRenderer) #Local variable only

file(GLOB_RECURSE PROJECT_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Header/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp
)

add_library(${LOW_NAME})

target_sources(${LOW_NAME} PRIVATE ${PROJECT_FILES})

target_include_directories(${LOW_NAME} 
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Header
	PRIVATE ${RESOURCES_INCLUDE_DIR}
)

target_include_directories(${LOW_NAME} PRIVATE ${LIB_INCLUDE_DIR})
target_link_libraries(${LOW_NAME}
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
)
set(LOW_NAME ${LOW_NAME} PARENT_SCOPE) #outer scope variable only

set(LOW_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Header PARENT_SCOPE)
#This command sets the LIB_INCLUDE_DIR variable to the path of the 'include' directory
