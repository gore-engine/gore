cmake_minimum_required(VERSION 3.20)

project(gore)

option(BUILD_SHARED_LIBS "Build shared libraries" ON)
# TODO: check debug or something else
option(ENABLE_TEST "Enable gore engine test" ON)
option(USE_RPS "Use AMD Render Pipeline Shaders" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Target Platform Detection
if (WIN32)
    set(TARGET_PLATFORM "Windows")
elseif (APPLE)
    set(TARGET_PLATFORM "Darwin")
elseif (UNIX)
    set(TARGET_PLATFORM "Linux")
endif ()

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/ShaderCompile.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/Resources.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/RpslCompile.cmake)

# Configurations

set(COMMON_SOURCE_FOLDERS
        "Core"
        "FileSystem"
        "Math"
        "Memory"
        "Windowing"
        "Graphics"
        "Rendering"
        "Scene"
        "Object"
        "Input"
        "Utilities"
        "Test"
)

if (${USE_RPS})
    list(APPEND COMMON_SOURCE_FOLDERS "Rendering/RPSL/Generated")
endif ()

set(COMMON_INCLUDE_FOLDERS "${CMAKE_CURRENT_SOURCE_DIR}")

if (${BUILD_SHARED_LIBS})
    list(APPEND COMMON_DEFINITIONS "ENGINE_DLL" "ENGINE_EXPORTS")
endif ()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    list(APPEND COMMON_DEFINITIONS "ENGINE_DEBUG=1")
endif ()

set (SHADER_SOURCE_FILES "Shaders/sample/cube.hlsl")
compile_shader(${SHADER_SOURCE_FILES} "vulkan" "vertex" "vs")
compile_shader(${SHADER_SOURCE_FILES} "vulkan" "pixel" "ps")

compile_shader("Shaders/sample/triangle.hlsl" "vulkan" "vertex" "vs")
compile_shader("Shaders/sample/triangle.hlsl" "vulkan" "pixel" "ps")

compile_shader("Shaders/sample/quad.hlsl" "vulkan" "vertex" "vs")
compile_shader("Shaders/sample/quad.hlsl" "vulkan" "pixel" "ps")

compile_shader("Shaders/sample/UnLit.hlsl" "vulkan" "vertex" "vs")
compile_shader("Shaders/sample/UnLit.hlsl" "vulkan" "pixel" "ps")

compile_shader("Shaders/sample/shadowmap.hlsl" "vulkan" "vertex" "vs")
compile_shader("Shaders/sample/shadowmap.hlsl" "vulkan" "pixel" "ps")

compile_rpsl_file("hello_triangle")

# Platform Specific Configurations

if (${TARGET_PLATFORM} STREQUAL "Windows")
    set(PLATFORM_SOURCE_FOLDERS "Platform/Windows")
    set(PLATFORM_INCLUDE_DIRECTORIES)
    set(PLATFORM_LIBRARIES)
    set(PLATFORM_DEFINITIONS "WIN32_LEAN_AND_MEAN" "NOMINMAX")
elseif (${TARGET_PLATFORM} STREQUAL "Linux")
    find_package(X11 REQUIRED)

    set(PLATFORM_SOURCE_FOLDERS "Platform/Linux" "Platform/CommonUnix")
    set(PLATFORM_INCLUDE_DIRECTORIES ${X11_INCLUDE_DIR})
    set(PLATFORM_LIBRARIES ${X11_LIBRARIES})
    set(PLATFORM_DEFINITIONS)
elseif (${TARGET_PLATFORM} STREQUAL "Darwin")
    find_library(COCOA Cocoa)
    find_library(METAL Metal)
    find_library(METALKIT MetalKit)
    find_library(APPKIT AppKit)
    find_library(QUARTZCORE QuartzCore)
    find_library(FOUNDATION Foundation)
    find_library(IOKIT IOKit)

    set(CMAKE_MACOSX_BUNDLE TRUE)

    set(PLATFORM_SOURCE_FOLDERS "Platform/macOS" "Platform/CommonUnix")
    set(PLATFORM_INCLUDE_DIRECTORIES)
    set(PLATFORM_LIBRARIES ${COCOA} ${METAL} ${METALKIT} ${APPKIT} ${QUARTZCORE} ${FOUNDATION} ${IOKIT})
    set(PLATFORM_DEFINITIONS)
endif ()

# Sources

set(COMMON_SOURCES "Prefix.h" "Export.h")
foreach (folder ${COMMON_SOURCE_FOLDERS})
    file(GLOB_RECURSE SOURCES "${folder}/*.cpp" "${folder}/*.inl" "${folder}/*.h" "${folder}/*.c")
    list(APPEND COMMON_SOURCES ${SOURCES})
endforeach ()

file(GLOB COMMON_PLATFORM_SOURCES "Platform/*.cpp" "Platform/*.h")
list(APPEND COMMON_SOURCES ${COMMON_PLATFORM_SOURCES})

set(PLATFORM_SOURCES)
foreach (folder ${PLATFORM_SOURCE_FOLDERS})
    file(GLOB_RECURSE SOURCES "${folder}/*.cpp" "${folder}/*.mm" "${folder}/*.h")
    list(APPEND PLATFORM_SOURCES ${SOURCES})
endforeach ()

# Libraries

# glfw
find_package(glfw3 CONFIG REQUIRED)
list(APPEND COMMON_LIBRARIES glfw)
list(APPEND COMMON_DEFINITIONS "GLFW_INCLUDE_NONE")

# vulkan
find_package(Vulkan REQUIRED)
list(APPEND COMMON_INCLUDE_FOLDERS ${Vulkan_INCLUDE_DIRS})

# vulkan memory allocator
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
list(APPEND COMMON_LIBRARIES GPUOpen::VulkanMemoryAllocator)

# imgui
find_package(imgui CONFIG REQUIRED)
list(APPEND COMMON_LIBRARIES imgui::imgui)

# stb
find_package(Stb REQUIRED)
list(APPEND COMMON_INCLUDE_FOLDERS ${Stb_INCLUDE_DIR})

# assimp
find_package(assimp CONFIG REQUIRED)
list(APPEND COMMON_LIBRARIES assimp::assimp)

# tinygltf
find_path(TINYGLTF_INCLUDE_DIRS "tiny_gltf.h")
list(APPEND COMMON_INCLUDE_FOLDERS ${TINYGLTF_INCLUDE_DIRS})

# Manually add RealtimeMath
add_subdirectory(External/rtm)
list(APPEND COMMON_LIBRARIES RealtimeMath)

# Gore AMD Render Pipeline Shaders
add_subdirectory(External/AMDRenderPipelineShaders)
list(APPEND COMMON_LIBRARIES AMDRps)

# catch2
if (${ENABLE_TEST})
    find_package(Catch2 3 REQUIRED)
    
    list(APPEND COMMON_LIBRARIES Catch2::Catch2WithMain)
    list(APPEND COMMON_DEFINITIONS "ENABLE_TEST")

    # These tests can use the Catch2-provided main
    add_executable(${PROJECT_NAME}Test ${COMMON_SOURCES} ${PLATFORM_SOURCES})
    
    target_include_directories(${PROJECT_NAME}Test PRIVATE ${COMMON_INCLUDE_FOLDERS} ${PLATFORM_INCLUDE_DIRECTORIES})
    target_compile_definitions(${PROJECT_NAME}Test PRIVATE ${COMMON_DEFINITIONS})
    target_link_libraries(${PROJECT_NAME}Test PRIVATE ${COMMON_LIBRARIES} ${PLATFORM_LIBRARIES})
    
    include(CTest)
    include(Catch)
    catch_discover_tests(${PROJECT_NAME}Test WORKING_DIRECTORY "$<TARGET_FILE_DIR:${PROJECT_NAME}Test>")
endif()

# Targets

add_library(${PROJECT_NAME} ${COMMON_SOURCES} ${PLATFORM_SOURCES})

add_shader_dependencies(${PROJECT_NAME})
add_rpsl_dependencies(${PROJECT_NAME})
copy_texture_resources(${PROJECT_NAME})
copy_gltf_resources(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC ${COMMON_INCLUDE_FOLDERS} ${PLATFORM_INCLUDE_DIRECTORIES})
target_compile_definitions(${PROJECT_NAME} PRIVATE ${COMMON_DEFINITIONS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${COMMON_LIBRARIES} ${PLATFORM_LIBRARIES})

# Vulkan Post Build copy
if (${TARGET_PLATFORM} STREQUAL "Darwin")
    # get directory of libvulkan.dylib from Vulkan_LIBRARIES
    get_filename_component(VULKAN_LIB_DIR ${Vulkan_LIBRARIES} DIRECTORY)

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            "${VULKAN_LIB_DIR}/libMoltenVK.dylib"
            $<TARGET_FILE_DIR:${PROJECT_NAME}>/../lib/libMoltenVK.dylib)

    file(GLOB VK_LAYER_LIBS "${VULKAN_LIB_DIR}/libVkLayer*.dylib")
    foreach (VK_LAYER_LIB ${VK_LAYER_LIBS})
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${VK_LAYER_LIB}"
                $<TARGET_FILE_DIR:${PROJECT_NAME}>/../lib)
    endforeach ()

    file(GLOB VULKAN_LIBS "${VULKAN_LIB_DIR}/libvulkan*.dylib")
    foreach (VULKAN_LIB ${VULKAN_LIBS})
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${VULKAN_LIB}"
                $<TARGET_FILE_DIR:${PROJECT_NAME}>/../Frameworks)
    endforeach ()

    # copy the folder containing icd and explicit layers
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${VULKAN_LIB_DIR}/../share/vulkan/icd.d"
            $<TARGET_FILE_DIR:${PROJECT_NAME}>/../Resources/vulkan/icd.d)

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${VULKAN_LIB_DIR}/../share/vulkan/explicit_layer.d"
            $<TARGET_FILE_DIR:${PROJECT_NAME}>/../Resources/vulkan/explicit_layer.d)
endif ()
