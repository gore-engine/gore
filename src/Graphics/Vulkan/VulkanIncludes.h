#pragma once

#include "Prefix.h"

#include "Core/Log.h"

#define VK_NO_PROTOTYPES
#define VK_ENABLE_BETA_EXTENSIONS

#if PLATFORM_WIN
    #define VK_USE_PLATFORM_WIN32_KHR
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
#elif PLATFORM_ANDROID
    #define VK_USE_PLATFORM_ANDROID_KHR
#elif PLATFORM_LINUX
    #define VK_USE_PLATFORM_XLIB_KHR
#elif PLATFORM_MACOS
    #define VK_USE_PLATFORM_METAL_EXT
#else
    #error Unsupported platform
#endif

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_extension_inspection.hpp>
#include <vulkan/vulkan_format_traits.hpp>
#include <vulkan/vulkan_funcs.hpp>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_hash.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan_static_assertions.hpp>
#include <vulkan/vulkan_structs.hpp>
#include <vulkan/vulkan_to_string.hpp>

#ifdef VK_NO_PROTOTYPES
    #define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#endif
#include <vk_mem_alloc.h>

#if !ENGINE_DEBUG

    #define VK_CHECK_RESULT(x) \
        do {                   \
        } while (0)

#else

    #define VK_CHECK_RESULT(x)                                                                                    \
        do {                                                                                                      \
            if (x < 0)                                                                                            \
            {                                                                                                     \
                LOG_STREAM(ERROR) << "Vulkan error " << ::vk::to_string(static_cast<vk::Result>(x)) << std::endl; \
            }                                                                                                     \
        } while (0)

#endif
