cmake_minimum_required(VERSION 3.20)

include(cmake/TargetArch.cmake)

if (WIN32)
    set(VCPKG_CMAKE_SYSTEM_NAME "windows")
elseif (UNIX AND NOT APPLE)
    set(VCPKG_CMAKE_SYSTEM_NAME "linux")
elseif (APPLE)
    set(VCPKG_CMAKE_SYSTEM_NAME "osx")
endif ()

target_architecture(VCPKG_TARGET_ARCHITECTURE)
set(VCPKG_TARGET_TRIPLET "${VCPKG_TARGET_ARCHITECTURE}-${VCPKG_CMAKE_SYSTEM_NAME}")
if (NOT ${VCPKG_TARGET_ARCHITECTURE} STREQUAL "x64")
    set($(VCPKG_FORCE_SYSTEM_BINARIES) ON)
endif ()

set(VCPKG_MANIFEST_MODE ON CACHE BOOL "" FORCE)

if (DEFINED ${CMAKE_TOOLCHAIN_FILE})
    set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE ${CMAKE_TOOLCHAIN_FILE})
endif ()

set(VCPKG_PARENT_DIR ${CMAKE_CURRENT_BINARY_DIR})
include(cmake/vcpkg.cmake)

# workaround for using toolchain file from vcpkg
include(${VCPKG_PARENT_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake)

project(gore_engine)

add_subdirectory(sample)