#pragma once

#include "VulkanIncludes.h"
#include "VulkanInstance.h"
#include "VulkanExtensions.h"

#include <vector>
#include <string>

namespace gore
{

struct VulkanPhysicalDevice
{
    int index;
    VkPhysicalDevice physicalDevice;
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceMemoryProperties memoryProperties;
    std::vector<VkQueueFamilyProperties> queueFamilyProperties;
    std::vector<bool> queueFamilySupportsPresent;
    std::vector<VkExtensionProperties> extensionProperties;

public:
    VulkanPhysicalDevice(VulkanInstance* instance, int index, VkPhysicalDevice vkPhysicalDevice);

    void LogInfo() const;
    [[nodiscard]] int Score() const;

    [[nodiscard]] bool QueueFamilyIsPresentable(uint32_t queueFamilyIndex, void* nativeWindowHandle) const;

    VulkanInstance* instance;
};

class VulkanDevice
{
public:
    VulkanDevice(VulkanInstance* instance, VulkanPhysicalDevice physicalDevice);
    ~VulkanDevice();

    [[nodiscard]] VkDevice GetDevice() const { return m_Device; }
    [[nodiscard]] VkPhysicalDevice GetPhysicalDevice() const { return m_PhysicalDevice.physicalDevice; }
    [[nodiscard]] const VulkanPhysicalDevice& GetPhysicalDeviceProperties() const { return m_PhysicalDevice; }
    [[nodiscard]] bool HasExtension(VulkanDeviceExtension extension) const;

    VolkDeviceTable API;

private:
    VulkanInstance* m_Instance;

    VkDevice m_Device;
    VulkanPhysicalDevice m_PhysicalDevice;
    VulkanDeviceExtensionBitset m_EnabledExtensions;

    std::vector<std::vector<VkQueue>> m_Queues;
};

} // namespace gore
