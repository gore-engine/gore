cmake_minimum_required(VERSION 3.20)

include(cmake/TargetArch.cmake)

# Target Platform Detection
if(WIN32)
   set(TARGET_PLATFORM "Windows")
elseif(UNIX AND NOT APPLE)
   set(TARGET_PLATFORM "Linux")
elseif(APPLE)
   set(TARGET_PLATFORM "Darwin")
endif()

if(${TARGET_PLATFORM} STREQUAL "Windows")
   set(VCPKG_CMAKE_SYSTEM_NAME "windows")
elseif(${TARGET_PLATFORM} STREQUAL "Linux")
   set(VCPKG_CMAKE_SYSTEM_NAME "linux")
elseif(${TARGET_PLATFORM} STREQUAL "Darwin")
   set(VCPKG_CMAKE_SYSTEM_NAME "osx")
endif()

target_architecture(VCPKG_TARGET_ARCHITECTURE)
set(VCPKG_TARGET_TRIPLET "${VCPKG_TARGET_ARCHITECTURE}-${VCPKG_CMAKE_SYSTEM_NAME}")

set(VCPKG_PARENT_DIR "${CMAKE_CURRENT_BINARY_DIR}/")
set(VCPKG_MANIFEST_MODE ON CACHE BOOL "" FORCE)

include(cmake/vcpkg.cmake)
#vcpkg_install_manifest()

# workaround for using toolchain file from vcpkg
include(${CMAKE_TOOLCHAIN_FILE})

project(gore_engine)

add_subdirectory(sample)