cmake_minimum_required(VERSION 3.20)

project(gore)

option(BUILD_SHARED_LIBS "Build shared libraries" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Target Platform Detection
if (WIN32)
    set(TARGET_PLATFORM "Windows")
elseif (UNIX AND NOT APPLE)
    set(TARGET_PLATFORM "Linux")
elseif (APPLE)
    set(TARGET_PLATFORM "Darwin")
endif ()

# Configurations

set(COMMON_SOURCE_FOLDERS
        "Core"
        "Windowing"
        "Rendering"
        "Scene"
        "Object"
        "Utilities"
)
set(COMMON_INCLUDE_FOLDERS "${CMAKE_CURRENT_SOURCE_DIR}")

if (${BUILD_SHARED_LIBS})
    list(APPEND COMMON_DEFINITIONS "ENGINE_DLL" "ENGINE_EXPORTS")
endif ()

# Platform Specific Configurations

if (${TARGET_PLATFORM} STREQUAL "Windows")
    set(PLATFORM_SOURCE_FOLDERS "Platform/Windows")
    set(PLATFORM_INCLUDE_DIRECTORIES)
    set(PLATFORM_LIBRARIES)
    set(PLATFORM_DEFINITIONS "WIN32_LEAN_AND_MEAN" "NOMINMAX")
elseif (${TARGET_PLATFORM} STREQUAL "Linux")
    find_package(X11 REQUIRED)

    set(PLATFORM_SOURCE_FOLDERS "Platform/Linux")
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

    set(PLATFORM_SOURCE_FOLDERS "Platform/macOS")
    set(PLATFORM_INCLUDE_DIRECTORIES)
    set(PLATFORM_LIBRARIES ${COCOA} ${METAL} ${METALKIT} ${APPKIT} ${QUARTZCORE} ${FOUNDATION} ${IOKIT})
    set(PLATFORM_DEFINITIONS)
endif ()

# Sources

set(COMMON_SOURCES "Prefix.h" "Export.h")
foreach (folder ${COMMON_SOURCE_FOLDERS})
    file(GLOB_RECURSE SOURCES "${folder}/*.cpp" "${folder}/*.h")
    list(APPEND COMMON_SOURCES ${SOURCES})
endforeach ()

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

# glm
find_package(glm CONFIG REQUIRED)
list(APPEND COMMON_LIBRARIES glm::glm)

# vulkan
find_package(Vulkan REQUIRED)
list(APPEND COMMON_LIBRARIES Vulkan::Headers)

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

# cgltf
find_path(CGLTF_INCLUDE_DIRS "cgltf.h")
list(APPEND COMMON_INCLUDE_FOLDERS ${CGLTF_INCLUDE_DIRS})

# Targets

add_library(${PROJECT_NAME} ${COMMON_SOURCES} ${PLATFORM_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE ${COMMON_INCLUDE_FOLDERS} ${PLATFORM_INCLUDE_DIRECTORIES})
target_compile_definitions(${PROJECT_NAME} PRIVATE ${COMMON_DEFINITIONS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${COMMON_LIBRARIES} ${PLATFORM_LIBRARIES})

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
