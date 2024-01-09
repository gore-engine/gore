#include "Prefix.h"

#include "VulkanExtensions.h"

#include <map>

namespace gore
{

const char* GetVulkanExtensionName(VulkanInstanceExtension extension)
{
    switch (extension)
    {
#define INSTANCE_EXTENSION(e)     \
    case VulkanInstanceExtension::k##e: \
        return #e;
#define DEVICE_EXTENSION(e)

#include "VulkanExtensions.inl"

#undef INSTANCE_EXTENSION
#undef DEVICE_EXTENSION
        default:
            return "Unknown";
    }
}

const char* GetVulkanExtensionName(VulkanDeviceExtension extension)
{
    switch (extension)
    {
#define INSTANCE_EXTENSION(e)
#define DEVICE_EXTENSION(e)     \
    case VulkanDeviceExtension::k##e: \
        return #e;

#include "VulkanExtensions.inl"

#undef INSTANCE_EXTENSION
#undef DEVICE_EXTENSION
        default:
            return "Unknown";
    }
}

} // namespace gore