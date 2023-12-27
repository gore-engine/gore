#pragma once

#include "Prefix.h"

#include "Core/Log.h"

#define VK_NO_PROTOTYPES
#define VK_ENABLE_BETA_EXTENSIONS

#if PLATFORM_WIN
    #define VK_USE_PLATFORM_WIN32_KHR
#elif PLATFORM_ANDROID
    #define VK_USE_PLATFORM_ANDROID_KHR
#elif PLATFORM_LINUX
    #define VK_USE_PLATFORM_XLIB_KHR
#elif PLATFORM_MACOS
    #define VK_USE_PLATFORM_METAL_EXT
#else
    #error Unsupported platform
#endif

#include <volk.h>

#if !ENGINE_DEBUG

    #define VK_CHECK_RESULT(x) \
        do {                   \
        } while (0)

#else
namespace gore
{
const char* VkResultToString(VkResult result);
}

    #define VK_CHECK_RESULT(x)                                                             \
        do {                                                                               \
            if (x < 0)                                                                     \
            {                                                                              \
                LOG(ERROR, "Vulkan error %s (%d)\n", ::gore::VkResultToString(x), (int)x); \
            }                                                                              \
        } while (0)

#endif
