#pragma once

// platform detection
#if defined(_WIN32) || defined(__CYGWIN__)
    #define PLATFORM_WIN   1
    #define PLATFORM_LINUX 0
    #define PLATFORM_MACOS 0
#elif defined(__linux__)
    #define PLATFORM_WIN   0
    #define PLATFORM_LINUX 1
    #define PLATFORM_MACOS 0
#elif defined(__APPLE__)
    #define PLATFORM_WIN   0
    #define PLATFORM_LINUX 0
    #define PLATFORM_MACOS 1
#else
    #error "Unknown platform"
#endif

// compiler detection
#if defined(__clang__)
    #define COMPILER_CLANG 1
    #define COMPILER_GCC   1
#elif defined(__GNUC__)
    #define COMPILER_GCC 1
#elif defined(_MSC_VER)
    #define COMPILER_MSVC 1
#else
    #error "Unknown compiler"
#endif

// architecture detection
#if defined(_M_X64) || defined(__x86_64__)
    #define ARCH_X64 1
#elif defined(_M_IX86) || defined(__i386__)
    #define ARCH_X86 1
#elif defined(_M_ARM64) || defined(__aarch64__)
    #define ARCH_ARM64 1
#elif defined(_M_ARM) || defined(__arm__)
    #define ARCH_ARM 1
#else
    #error "Unknown architecture"
#endif
