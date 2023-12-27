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

// Instance Extensions
std::vector<VkExtensionProperties> GetAvailableInstanceExtensions(const std::vector<const char*>& layerNames);
std::vector<const char*> GetEnabledInstanceExtensions(VulkanInstanceExtensionBitset& instanceExtensionBitset,
                                                      const std::vector<VkLayerProperties>& availableLayers,
                                                      const std::vector<VkExtensionProperties>& availableExtensions);

// Device Extensions
std::vector<VkExtensionProperties> GetAvailableDeviceExtensions(VkPhysicalDevice physicalDevice);
std::vector<const char*> GetEnabledDeviceExtensions(VulkanDeviceExtensionBitset& deviceExtensionBitset,
                                                    const std::vector<VkExtensionProperties>& availableExtensions);

} // namespace gore
