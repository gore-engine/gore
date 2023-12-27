#pragma once

#include "Prefix.h"

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
