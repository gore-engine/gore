# dxc
# FindVulkan with dxc component emits warning on Windows Debug setup
if (CMAKE_VERSION VERSION_GREATER_EQUAL 3.25 AND NOT WIN32)
    find_package(Vulkan REQUIRED COMPONENTS dxc)
    set(VULKAN_DXC_EXECUTABLE ${Vulkan_dxc_EXECUTABLE})
else ()
    find_package(Vulkan REQUIRED)
    # try to find dxc executable in (and only in) glslangValidator directory
    get_filename_component(GLSLANG_VALIDATOR_DIR ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE} DIRECTORY)
    find_program(VULKAN_DXC_EXECUTABLE NAMES dxc PATHS ${GLSLANG_VALIDATOR_DIR} NO_CACHE NO_DEFAULT_PATH)
endif ()

if (VULKAN_DXC_EXECUTABLE STREQUAL "")
    message(FATAL_ERROR "dxc (from Vulkan SDK) not found! It is needed for shader compilation. Try installing Vulkan SDK with dxc.")
endif ()

# spirv-cross
find_package(spirv_cross_core CONFIG REQUIRED)
find_program(SPIRV_CROSS_EXECUTABLE NAMES spirv-cross REQUIRED)

file(REMOVE_RECURSE ${CMAKE_CURRENT_BINARY_DIR}/shaders)

# convert shader stage names to DirectX convention
function(DIRECTX_SHADER_STAGE SHADER_STAGE OUTPUT_DIRECTX_STAGE)
    if(${SHADER_STAGE} STREQUAL "vertex" OR ${SHADER_STAGE} STREQUAL "vert" OR ${SHADER_STAGE} STREQUAL "vs")
        set(${OUTPUT_DIRECTX_STAGE} "vs" PARENT_SCOPE)
    elseif(${SHADER_STAGE} STREQUAL "fragment" OR ${SHADER_STAGE} STREQUAL "frag" OR ${SHADER_STAGE} STREQUAL "pixel" OR ${SHADER_STAGE} STREQUAL "ps")
        set(${OUTPUT_DIRECTX_STAGE} "ps" PARENT_SCOPE)
    elseif(${SHADER_STAGE} STREQUAL "geometry" OR ${SHADER_STAGE} STREQUAL "geom" OR ${SHADER_STAGE} STREQUAL "gs")
        set(${OUTPUT_DIRECTX_STAGE} "gs" PARENT_SCOPE)
    elseif(${SHADER_STAGE} STREQUAL "tessellation_control" OR ${SHADER_STAGE} STREQUAL "tess_control" OR ${SHADER_STAGE} STREQUAL "tesc" OR ${SHADER_STAGE} STREQUAL "hull" OR ${SHADER_STAGE} STREQUAL "hs")
        set(${OUTPUT_DIRECTX_STAGE} "hs" PARENT_SCOPE)
    elseif(${SHADER_STAGE} STREQUAL "tessellation_evaluation" OR ${SHADER_STAGE} STREQUAL "tess_evaluation" OR ${SHADER_STAGE} STREQUAL "tese" OR ${SHADER_STAGE} STREQUAL "domain" OR ${SHADER_STAGE} STREQUAL "ds")
        set(${OUTPUT_DIRECTX_STAGE} "ds" PARENT_SCOPE)
    elseif(${SHADER_STAGE} STREQUAL "compute" OR ${SHADER_STAGE} STREQUAL "comp" OR ${SHADER_STAGE} STREQUAL "cs")
        set(${OUTPUT_DIRECTX_STAGE} "cs" PARENT_SCOPE)
    else()
        message(FATAL_ERROR "Unknown shader stage: ${SHADER_STAGE}")
    endif()
endfunction()

# convert shader stage names to OpenGL convention
function(OPENGL_SHADER_STAGE SHADER_STAGE OUTPUT_OPENGL_STAGE)
    if(${SHADER_STAGE} STREQUAL "vertex" OR ${SHADER_STAGE} STREQUAL "vert" OR ${SHADER_STAGE} STREQUAL "vs")
        set(${OUTPUT_OPENGL_STAGE} "vert" PARENT_SCOPE)
    elseif(${SHADER_STAGE} STREQUAL "fragment" OR ${SHADER_STAGE} STREQUAL "frag" OR ${SHADER_STAGE} STREQUAL "pixel" OR ${SHADER_STAGE} STREQUAL "ps")
        set(${OUTPUT_OPENGL_STAGE} "frag" PARENT_SCOPE)
    elseif(${SHADER_STAGE} STREQUAL "geometry" OR ${SHADER_STAGE} STREQUAL "geom" OR ${SHADER_STAGE} STREQUAL "gs")
        set(${OUTPUT_OPENGL_STAGE} "geom" PARENT_SCOPE)
    elseif(${SHADER_STAGE} STREQUAL "tessellation_control" OR ${SHADER_STAGE} STREQUAL "tess_control" OR ${SHADER_STAGE} STREQUAL "tesc" OR ${SHADER_STAGE} STREQUAL "hull" OR ${SHADER_STAGE} STREQUAL "hs")
        set(${OUTPUT_OPENGL_STAGE} "tesc" PARENT_SCOPE)
    elseif(${SHADER_STAGE} STREQUAL "tessellation_evaluation" OR ${SHADER_STAGE} STREQUAL "tess_evaluation" OR ${SHADER_STAGE} STREQUAL "tese" OR ${SHADER_STAGE} STREQUAL "domain" OR ${SHADER_STAGE} STREQUAL "ds")
        set(${OUTPUT_OPENGL_STAGE} "tese" PARENT_SCOPE)
    elseif(${SHADER_STAGE} STREQUAL "compute" OR ${SHADER_STAGE} STREQUAL "comp" OR ${SHADER_STAGE} STREQUAL "cs")
        set(${OUTPUT_OPENGL_STAGE} "comp" PARENT_SCOPE)
    else()
        message(FATAL_ERROR "Unknown shader stage: ${SHADER_STAGE}")
    endif()
endfunction()

# determine shader binary type based on graphics API
function(SHADER_BINARY_TYPE GRAPHICS_API OUTPUT_BINARY_TYPE)
    if(${GRAPHICS_API} STREQUAL "vulkan")
        set(${OUTPUT_BINARY_TYPE} "spirv" PARENT_SCOPE)
    elseif(${GRAPHICS_API} STREQUAL "opengl")
        set(${OUTPUT_BINARY_TYPE} "glsl" PARENT_SCOPE)
    elseif(${GRAPHICS_API} STREQUAL "directx")
        set(${OUTPUT_BINARY_TYPE} "dxil" PARENT_SCOPE)
    elseif(${GRAPHICS_API} STREQUAL "metal")
        set(${OUTPUT_BINARY_TYPE} "msl" PARENT_SCOPE)
    else()
        message(FATAL_ERROR "Unknown graphics API: ${GRAPHICS_API}")
    endif()
endfunction()

# determine shader source type based on file extension
function(SHADER_SOURCE_TYPE INPUT_FILE OUTPUT_SOURCE_TYPE)
    get_filename_component(INPUT_FILE_EXTENSION ${INPUT_FILE} EXT)
    if(${INPUT_FILE_EXTENSION} STREQUAL ".hlsl")
        set(${OUTPUT_SOURCE_TYPE} "hlsl" PARENT_SCOPE)
    elseif(${INPUT_FILE_EXTENSION} STREQUAL ".glsl")
        set(${OUTPUT_SOURCE_TYPE} "glsl" PARENT_SCOPE)
    else()
        message(FATAL_ERROR "Unknown shader source type: ${INPUT_FILE_EXTENSION}")
    endif()
endfunction()

# get output directory for shader, keeping the same directory structure as in Shaders directory
function(SHADER_OUTPUT_DIR INPUT_FILE OUTPUT_DIR)
    get_filename_component(INPUT_FILE_DIR ${INPUT_FILE} DIRECTORY)
    get_filename_component(INPUT_FILE_DIR ${INPUT_FILE_DIR} ABSOLUTE)
    file(RELATIVE_PATH RELATIVE_PATH_TO_SHADER ${CMAKE_CURRENT_SOURCE_DIR}/Shaders ${INPUT_FILE_DIR})
    set(${OUTPUT_DIR} ${CMAKE_CURRENT_BINARY_DIR}/shaders/${RELATIVE_PATH_TO_SHADER} PARENT_SCOPE)
endfunction()

# get output file for shader with given extension
function(SHADER_OUTPUT_FILE INPUT_FILE TARGET_EXTENSION OUTPUT_FILE)
    SHADER_OUTPUT_DIR(${INPUT_FILE} OUTPUT_DIR)
    get_filename_component(INPUT_FILE_NAME ${INPUT_FILE} NAME_WE)
    set(${OUTPUT_FILE} ${OUTPUT_DIR}/${INPUT_FILE_NAME}.${TARGET_EXTENSION} PARENT_SCOPE)
endfunction()

##################################################
#                 Compilations                   #
##################################################

# compile HLSL shader to SPIR-V
function(HLSL_TO_SPIRV INPUT_HLSL SHADER_STAGE ENTRY_POINT OUTPUT_BINARY)
    DIRECTX_SHADER_STAGE(${SHADER_STAGE} DIRECTX_STAGE)
    OPENGL_SHADER_STAGE(${SHADER_STAGE} OPENGL_STAGE)
    SHADER_OUTPUT_FILE(${INPUT_HLSL} ${OPENGL_STAGE}.spv OUTPUT_SPIRV)
    set(${OUTPUT_BINARY} ${OUTPUT_SPIRV} PARENT_SCOPE)
    get_filename_component(OUTPUT_DIR ${OUTPUT_SPIRV} DIRECTORY)
    add_custom_command(
        OUTPUT ${OUTPUT_SPIRV}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_DIR}
        COMMAND ${VULKAN_DXC_EXECUTABLE} -spirv -T ${DIRECTX_STAGE}_6_0 -E ${ENTRY_POINT} -Fo ${OUTPUT_SPIRV} ${INPUT_HLSL} -DENABLE_SPIRV_CODEGEN=ON
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS ${INPUT_HLSL}
    )
endfunction()



# shader compilation with automatic source&binary type detection
function(compile_shader INPUT_FILE GRAPHICS_API SHADER_STAGE ENTRY_POINT)
    SHADER_BINARY_TYPE(${GRAPHICS_API} BINARY_TYPE)
    SHADER_SOURCE_TYPE(${INPUT_FILE} SOURCE_TYPE)

    if (${SOURCE_TYPE} STREQUAL "hlsl" AND ${BINARY_TYPE} STREQUAL "spirv")
        HLSL_TO_SPIRV(${INPUT_FILE} ${SHADER_STAGE} ${ENTRY_POINT} SHADER_BINARY)
    else()
        message(FATAL_ERROR "Unsupported shader source type: ${SOURCE_TYPE} and binary type: ${BINARY_TYPE}")
    endif()

    list(APPEND SHADER_BINARY_OUTPUT ${SHADER_BINARY})
    set(SHADER_BINARY_OUTPUT ${SHADER_BINARY_OUTPUT} PARENT_SCOPE)
endfunction()

# add shader dependencies to specified target
function(add_shader_dependencies PROJECT_TARGET)
    if (NOT SHADER_BINARY_OUTPUT)
        return()
    endif ()

    add_custom_target(Shaders ALL DEPENDS ${SHADER_BINARY_OUTPUT})
    add_dependencies(${PROJECT_TARGET} Shaders)

    set(SHADER_INTERMEDIATE_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders)

    if (WIN32)
        set(SHADER_FINAL_OUTPUT_DIR $<TARGET_FILE_DIR:${PROJECT_NAME}>/Resources/Shaders)
    elseif (APPLE)
        set(SHADER_FINAL_OUTPUT_DIR $<TARGET_FILE_DIR:${PROJECT_NAME}>/../Resources/Shaders)
    elseif (UNIX)
        set(SHADER_FINAL_OUTPUT_DIR $<TARGET_FILE_DIR:${PROJECT_NAME}>/../bin/Resources/Shaders)
    endif ()

    add_custom_command(TARGET ${PROJECT_TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E remove_directory ${SHADER_FINAL_OUTPUT_DIR}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADER_FINAL_OUTPUT_DIR}
    )

    foreach (SHADER_BINARY ${SHADER_BINARY_OUTPUT})
        # get relative path of SHADER_BINARY_OUTPUT to SHADER_INTERMEDIATE_OUTPUT_DIR
        file(RELATIVE_PATH SHADER_BINARY_RELATIVE_PATH ${SHADER_INTERMEDIATE_OUTPUT_DIR} ${SHADER_BINARY})

        add_custom_command(TARGET ${PROJECT_TARGET} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${SHADER_BINARY}"
                "${SHADER_FINAL_OUTPUT_DIR}/${SHADER_BINARY_RELATIVE_PATH}"
        )
    endforeach ()

endfunction()