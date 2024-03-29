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

#if COMPILER_MSVC
    #define DECLTYPE __cdecl
#else
    #define DECLTYPE
#endif

// common includes
#include <cstdint>

#include "Utilities/Defines.h"

// some pre-includes and undefs to prevent future conflicts
#if PLATFORM_WIN
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
    #include <Windows.h>
#elif PLATFORM_LINUX
    #include <X11/Xlib.h>
#elif PLATFORM_MACOS

#endif

// windows
#ifdef ERROR
    #undef ERROR
#endif

#ifdef near
    #undef near
#endif

#ifdef far
    #undef far
#endif

#ifdef min
    #undef min
#endif

#ifdef max
    #undef max
#endif

// x11
#ifdef None
    #undef None
#endif

#ifdef Button1
    #undef Button1
#endif

#ifdef Button2
    #undef Button2
#endif

#ifdef Button3
    #undef Button3
#endif

#ifdef Button4
    #undef Button4
#endif

#ifdef Button5
    #undef Button5
#endif
