#include "Prefix.h"

#include "VulkanInstance.h"

#include "Core/Log.h"
#include "Platform/LoadLibrary.h"

namespace gore
{

static void* s_VulkanLibraryHandle = nullptr;

#if PLATFORM_ANDROID
const char* const kVulkanLibraryName = "libvulkan.so";
#elif PLATFORM_WIN
const char* const kVulkanLibraryName = "vulkan-1.dll";
#elif PLATFORM_LINUX
const char* const kVulkanLibraryName = "libvulkan.so.1";
#elif PLATFORM_MACOS
const char* const kVulkanLibraryName = "libvulkan.1.dylib";
#else
    #error Unsupported Platform
#endif

bool InitializeVulkanInstance(VkInstance* instance)
{
    if (s_VulkanLibraryHandle == nullptr)
    {
        s_VulkanLibraryHandle = LoadDynamicLibrary(kVulkanLibraryName);
        if (!s_VulkanLibraryHandle)
        {
            LOG(FATAL, "Failed to load Vulkan Library\n");
            return false;
        }
        LOG(INFO, "Loaded Vulkan Library <%s>\n", kVulkanLibraryName);
    }


    return true;
}

bool ShutdownVulkanInstance(VkInstance instance)
{


    if (s_VulkanLibraryHandle != nullptr)
    {
        UnloadDynamicLibrary(s_VulkanLibraryHandle);
        s_VulkanLibraryHandle = nullptr;
    }

    return true;
}

}