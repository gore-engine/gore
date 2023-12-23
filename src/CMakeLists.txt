cmake_minimum_required(VERSION 3.20)

project(gore)

option(BUILD_SHARED_LIBS "Build shared libraries" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
    set(TARGET_PLATFORM "Windows")
elseif(UNIX AND NOT APPLE)
    set(TARGET_PLATFORM "Linux")
elseif(APPLE)
    set(TARGET_PLATFORM "macOS")
endif()

# Libraries

# GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
set(GLFW_LIBRARY_TYPE SHARED CACHE STRING "" FORCE)
add_subdirectory(External/glfw glfw)

# Vulkan
set(VULKAN_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/External/Vulkan/include")

# Configurations

set(COMMON_SOURCE_FOLDERS "Core" "Windowing" "Rendering")
set(COMMON_INCLUDE_FOLDERS "${CMAKE_CURRENT_SOURCE_DIR}" "${VULKAN_INCLUDE_DIR}")
set(COMMON_LIBRARIES glfw)
set(COMMON_DEFINITIONS "GLFW_INCLUDE_NONE")

if(${BUILD_SHARED_LIBS})
    list(APPEND COMMON_DEFINITIONS "ENGINE_DLL" "ENGINE_EXPORTS")
endif()

# Platform Specific Configurations

if(${TARGET_PLATFORM} STREQUAL "Windows")
    set(PLATFORM_SOURCE_FOLDERS "Platform/Windows")
    set(PLATFORM_INCLUDE_DIRECTORIES)
    set(PLATFORM_LIBRARIES)
    set(PLATFORM_DEFINITIONS "WIN32_LEAN_AND_MEAN" "NOMINMAX")
elseif(${TARGET_PLATFORM} STREQUAL "Linux")
    find_package(X11 REQUIRED)

    set(PLATFORM_SOURCE_FOLDERS "Platform/Linux")
    set(PLATFORM_INCLUDE_DIRECTORIES ${X11_INCLUDE_DIR})
    set(PLATFORM_LIBRARIES ${X11_LIBRARIES})
    set(PLATFORM_DEFINITIONS)
elseif(${TARGET_PLATFORM} STREQUAL "macOS")
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
endif()

# Sources

set(COMMON_SOURCES "Prefix.h" "Export.h")
foreach(folder ${COMMON_SOURCE_FOLDERS})
    file(GLOB_RECURSE SOURCES "${folder}/*.cpp" "${folder}/*.h")
    list(APPEND COMMON_SOURCES ${SOURCES})
endforeach()

set(PLATFORM_SOURCES)
foreach(folder ${PLATFORM_SOURCE_FOLDERS})
    file(GLOB_RECURSE SOURCES "${folder}/*.cpp" "${folder}/*.mm" "${folder}/*.h")
    list(APPEND PLATFORM_SOURCES ${SOURCES})
endforeach()

# Targets

add_library(${PROJECT_NAME} ${COMMON_SOURCES} ${PLATFORM_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE ${COMMON_INCLUDE_FOLDERS} ${PLATFORM_INCLUDE_DIRECTORIES})
target_compile_definitions(${PROJECT_NAME} PRIVATE ${COMMON_DEFINITIONS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${COMMON_LIBRARIES} ${PLATFORM_LIBRARIES})
