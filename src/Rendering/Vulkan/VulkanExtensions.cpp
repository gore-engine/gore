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

template <typename EnumBitset, typename Enum>
static std::vector<const char*> BuildEnabledExtensions(const std::vector<VkExtensionProperties>& extensionProperties,
                                                       EnumBitset& requestedKnownExtensions)
{
    std::vector<const char*> enabledExtensions;

    // std::map because it's sorted and gives nicer log
    std::map<std::string, int> supportedExtensions;
    for (const VkExtensionProperties& property : extensionProperties)
        supportedExtensions[property.extensionName] = -1;

    std::vector<const char*> enabledInstanceExtensions;
    EnumBitset enabledKnownExtensions;
    enabledKnownExtensions.reset();
    for (int i = 0; i < static_cast<int>(Enum::COUNT); ++i)
    {
        const char* extensionName = GetVulkanExtensionName(static_cast<Enum>(i));

        auto it = supportedExtensions.find(extensionName);
        if (it != supportedExtensions.end())
        {
            if (requestedKnownExtensions.test(i))
            {
                it->second = i;
                enabledKnownExtensions.set(i);
                enabledExtensions.push_back(extensionName);
            }
        }
    }

    LOG(INFO, "extensions: count=%d\n", static_cast<int>(extensionProperties.size()));
    for (const auto& ext : supportedExtensions)
    {
        LOG(INFO, "extensions:   [ %s ] %s\n", ext.second == -1 ? " " : "O", ext.first.data());
    }

    requestedKnownExtensions = enabledKnownExtensions;

    return enabledExtensions;
}

std::vector<VkExtensionProperties> GetAvailableInstanceExtensions(const std::vector<const char*>& layerNames)
{
    std::vector<VkExtensionProperties> instanceExtensionProperties;

    std::vector<uint32_t> instanceExtensionPropertyCounts(layerNames.size() + 1);

    // First, get the count of all extensions of all layers
    uint32_t totalInstanceExtensionPropertyCount = 0;
    uint32_t instanceLayerExtensionPropertyCount = 0;

    VkResult res = vkEnumerateInstanceExtensionProperties(nullptr, &instanceLayerExtensionPropertyCount, nullptr);
    VK_CHECK_RESULT(res);
    instanceExtensionPropertyCounts[0] = instanceLayerExtensionPropertyCount;
    totalInstanceExtensionPropertyCount += instanceLayerExtensionPropertyCount;

    for (uint32_t i = 0; i < layerNames.size(); ++i)
    {
        instanceLayerExtensionPropertyCount = 0;

        res = vkEnumerateInstanceExtensionProperties(layerNames[i],
                                                     &instanceLayerExtensionPropertyCount,
                                                     nullptr);
        VK_CHECK_RESULT(res);
        instanceExtensionPropertyCounts[i + 1] = instanceLayerExtensionPropertyCount;
        totalInstanceExtensionPropertyCount += instanceLayerExtensionPropertyCount;
    }

    // Then, get all extensions of all layers
    if (totalInstanceExtensionPropertyCount > 0)
    {
        instanceExtensionProperties.resize(totalInstanceExtensionPropertyCount);
        uint32_t offset = 0;

        res = vkEnumerateInstanceExtensionProperties(nullptr,
                                                     &instanceExtensionPropertyCounts[0],
                                                     &instanceExtensionProperties[offset]);
        VK_CHECK_RESULT(res);
        offset += instanceExtensionPropertyCounts[0];

        for (uint32_t i = 0; i < layerNames.size(); ++i)
        {
            res = vkEnumerateInstanceExtensionProperties(layerNames[i],
                                                         &instanceExtensionPropertyCounts[i + 1],
                                                         &instanceExtensionProperties[offset]);
            VK_CHECK_RESULT(res);
            offset += instanceExtensionPropertyCounts[i + 1];
        }
    }

    return instanceExtensionProperties;
}

std::vector<const char*> GetEnabledInstanceExtensions(VulkanInstanceExtensionBitset& instanceExtensionBitset,
                                                      const std::vector<VkLayerProperties>& availableLayers,
                                                      const std::vector<VkExtensionProperties>& availableExtensions)
{
    instanceExtensionBitset.set();
#ifndef ENGINE_DEBUG
    instanceExtensionBitset.reset(static_cast<size_t>(VulkanInstanceExtension::kVK_EXT_debug_report));
    instanceExtensionBitset.reset(static_cast<size_t>(VulkanInstanceExtension::kVK_EXT_debug_utils));
#endif

    return BuildEnabledExtensions<VulkanInstanceExtensionBitset, VulkanInstanceExtension>(availableExtensions,
                                                                                          instanceExtensionBitset);
}

std::vector<VkExtensionProperties> GetAvailableDeviceExtensions(VkPhysicalDevice physicalDevice)
{
    std::vector<VkExtensionProperties> deviceExtensionProperties;

    uint32_t deviceExtensionPropertyCount = 0;

    VkResult res = vkEnumerateDeviceExtensionProperties(physicalDevice,
                                                        VK_NULL_HANDLE,
                                                        &deviceExtensionPropertyCount,
                                                        VK_NULL_HANDLE);
    VK_CHECK_RESULT(res);
    if (deviceExtensionPropertyCount > 0)
    {
        deviceExtensionProperties.resize(deviceExtensionPropertyCount);
        res = vkEnumerateDeviceExtensionProperties(physicalDevice,
                                                   VK_NULL_HANDLE,
                                                   &deviceExtensionPropertyCount,
                                                   deviceExtensionProperties.data());
        VK_CHECK_RESULT(res);
    }

    return deviceExtensionProperties;
}

std::vector<const char*> GetEnabledDeviceExtensions(VulkanDeviceExtensionBitset& deviceExtensionBitset,
                                                    const std::vector<VkExtensionProperties>& availableExtensions)
{
    deviceExtensionBitset.set();
    return BuildEnabledExtensions<VulkanDeviceExtensionBitset, VulkanDeviceExtension>(availableExtensions,
                                                                                      deviceExtensionBitset);
}

} // namespace gore