#Assets
#get_filename_component(TARGET_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

file(GLOB_RECURSE SHADER_FILES CONFIGURE_DEPENDS 
	${CMAKE_CURRENT_SOURCE_DIR}/Shaders/*.glsl
	${CMAKE_CURRENT_SOURCE_DIR}/Shaders/*.frag
	${CMAKE_CURRENT_SOURCE_DIR}/Shaders/*.vert
)

file(GLOB_RECURSE ASSET_FILES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/Meshes/**/*.obj
    ${CMAKE_CURRENT_SOURCE_DIR}/Meshes/**/*.tga
    ${CMAKE_CURRENT_SOURCE_DIR}/Meshes/**/*.jpg
    ${CMAKE_CURRENT_SOURCE_DIR}/Meshes/**/*.png
)

file(GLOB_RECURSE SOUND_FILES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/Sounds/**/*.wav
    ${CMAKE_CURRENT_SOURCE_DIR}/Sounds/**/*.mp3
    ${CMAKE_CURRENT_SOURCE_DIR}/Sounds/**/*.ogg
    ${CMAKE_CURRENT_SOURCE_DIR}/Sounds/**/*.mid
)

# add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
    # COMMAND ${CMAKE_COMMAND} -E copy_directory
        # "${CMAKE_CURRENT_SOURCE_DIR}/Shaders"
        # "$<TARGET_FILE_DIR:${TARGET_NAME}>/Shaders"
# )

# add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
    # COMMAND ${CMAKE_COMMAND} -E copy_directory
        # "${CMAKE_CURRENT_SOURCE_DIR}/Meshes"
        # "$<TARGET_FILE_DIR:${TARGET_NAME}>/Meshes"
# )

# add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
    # COMMAND ${CMAKE_COMMAND} -E copy_directory
        # "${CMAKE_CURRENT_SOURCE_DIR}/Sounds"
        # "$<TARGET_FILE_DIR:${TARGET_NAME}>/Sounds"
# )