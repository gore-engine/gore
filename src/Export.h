#pragma once

#include "Prefix.h"

#if PLATFORM_WIN
    #define ENGINE_DLL_EXPORT __declspec(dllexport)
    #define ENGINE_DLL_IMPORT __declspec(dllimport)
    #define ENGINE_DLL_LOCAL
    #define ENGINE_CALLCONV __stdcall
#else
    #define ENGINE_DLL_EXPORT __attribute__((visibility("default")))
    #define ENGINE_DLL_IMPORT __attribute__((visibility("default")))
    #define ENGINE_DLL_LOCAL  __attribute__((visibility("hidden")))
    #define ENGINE_CALLCONV
#endif

#if defined(__cplusplus)
    #define ENGINE_EXTERN_C extern "C"
#else
    #define ENGINE_EXTERN_C
#endif

#if defined(ENGINE_DLL)
    #if defined(ENGINE_EXPORTS)
        #define ENGINE_API ENGINE_DLL_EXPORT
    #else
        #define ENGINE_API ENGINE_DLL_IMPORT
    #endif
#else
    #define ENGINE_API
#endif

#define ENGINE_CLASS(CLASS_NAME)                     class ENGINE_API CLASS_NAME
#define ENGINE_STRUCT(STRUCT_NAME)                   struct ENGINE_API STRUCT_NAME

#define ENGINE_API_FUNC(RETURN_TYPE, FUNC_NAME, ...) ENGINE_EXTERN_C ENGINE_API RETURN_TYPE ENGINE_CALLCONV FUNC_NAME(__VA_ARGS__)
