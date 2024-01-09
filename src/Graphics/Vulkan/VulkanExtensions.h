#pragma once

#include "VulkanIncludes.h"

#include <bitset>
#include <vector>

namespace gore
{

enum class VulkanInstanceExtension : uint32_t
{
#define INSTANCE_EXTENSION(e) k##e,
#define DEVICE_EXTENSION(e)
#include "VulkanExtensions.inl"
#undef INSTANCE_EXTENSION
#undef DEVICE_EXTENSION
    COUNT
};

const char* GetVulkanExtensionName(VulkanInstanceExtension extension);

typedef std::bitset<static_cast<size_t>(VulkanInstanceExtension::COUNT)> VulkanInstanceExtensionBitset;

enum class VulkanDeviceExtension : uint32_t
{
#define INSTANCE_EXTENSION(e)
#define DEVICE_EXTENSION(e) k##e,
#include "VulkanExtensions.inl"
#undef INSTANCE_EXTENSION
#undef DEVICE_EXTENSION
    COUNT
};

const char* GetVulkanExtensionName(VulkanDeviceExtension extension);

typedef std::bitset<static_cast<size_t>(VulkanDeviceExtension::COUNT)> VulkanDeviceExtensionBitset;

template <typename EnumBitset, typename Enum>
static std::vector<const char*> BuildEnabledExtensions(const std::vector<vk::ExtensionProperties>& extensionProperties,
                                                       EnumBitset& requestedKnownExtensions)
{
    std::vector<const char*> enabledExtensions;

    // std::map because it's sorted and gives nicer log
    std::map<std::string, int> supportedExtensions;
    for (const vk::ExtensionProperties& property : extensionProperties)
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

    LOG(DEBUG, "extensions: count=%d\n", static_cast<int>(extensionProperties.size()));
    for (const auto& ext : supportedExtensions)
    {
        LOG(DEBUG, "extensions:   [ %s ] %s\n", ext.second == -1 ? " " : "O", ext.first.data());
    }

    requestedKnownExtensions = enabledKnownExtensions;

    return enabledExtensions;
}

} // namespace gore
