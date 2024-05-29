// WORKAROUND: would add api macro to shader compiler in the future
#define VULKAN_API (1)

#ifndef GORE_COMMON
#define GORE_COMMON

#ifdef VULKAN_API
    #include "./VulkanVariables.hlsl"
    #include "./VulkanBinding.hlsl"
#else
    #pragma error "API not supported"
#endif

#endif