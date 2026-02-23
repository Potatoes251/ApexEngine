set(LIB_NAME LibMath) #Local variable only

file(GLOB_RECURSE PROJECT_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Header/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp
)

add_library(${LIB_NAME})

target_sources(${LIB_NAME} PRIVATE ${PROJECT_FILES})

target_include_directories(${LIB_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Header)

set(LIB_NAME ${LIB_NAME} PARENT_SCOPE) #outer scope variable only

set(LIB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Header PARENT_SCOPE)
#This command sets the LIB_INCLUDE_DIR variable to the path of the 'include' directory