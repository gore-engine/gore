cmake_minimum_required(VERSION 3.20)

function(copy_texture_resources PROJECT_TARGET)
    cmake_path(GET CMAKE_CURRENT_SOURCE_DIR PARENT_PATH PARENT_DIR)
    set(INPUT_DIR ${PARENT_DIR}/resources/texture)
    set(FINAL_OUTPUT_DIR $<TARGET_FILE_DIR:${PROJECT_NAME}>/Resources/Textures)

    add_custom_command(TARGET ${PROJECT_TARGET} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${FINAL_OUTPUT_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${INPUT_DIR} ${FINAL_OUTPUT_DIR}
        COMMENT "Copying resources from ${INPUT_DIR} to ${FINAL_OUTPUT_DIR}"
    )
endfunction()