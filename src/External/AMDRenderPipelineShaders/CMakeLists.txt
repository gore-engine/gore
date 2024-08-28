project(AMDRps)

add_library(AMDRps INTERFACE)
target_include_directories(AMDRps INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include/)

set(AMDRpsLibDir ${CMAKE_CURRENT_SOURCE_DIR}/lib/Release)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(AMDRpsLibDir ${CMAKE_CURRENT_SOURCE_DIR}/lib/Debug)
endif ()

set(AMDRpsStaticLibs
    ${AMDRpsLibDir}/rps_core.lib
    ${AMDRpsLibDir}/rps_frontend.lib
    ${AMDRpsLibDir}/rps_runtime.lib
    ${AMDRpsLibDir}/rps_runtime_d3d11.lib
    ${AMDRpsLibDir}/rps_runtime_d3d12.lib
    ${AMDRpsLibDir}/rps_runtime_vk.lib
    ${AMDRpsLibDir}/rps_runtime_vkdyn.lib
)

target_link_libraries(AMDRps INTERFACE ${AMDRpsStaticLibs})