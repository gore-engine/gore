#pragma once

#include "VulkanIncludes.h"
#include "VulkanInstance.h"
#include "VulkanExtensions.h"
#include "VulkanQueue.h"

#include <vector>
#include <memory>
#include <mutex>

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

    // The result queue may or may not be the same one as the previous call since we are using round-robin.
    // So you need to make sure you properly synchronize the commands submitted to them.
    VulkanQueue GetQueue(VulkanQueueType type);

    VolkDeviceTable API;

private:
    VulkanInstance* m_Instance;

    VkDevice m_Device;
    VulkanPhysicalDevice m_PhysicalDevice;
    VulkanDeviceExtensionBitset m_EnabledExtensions;

    friend class VulkanQueue;
    std::vector<std::vector<VkQueue>> m_Queues;
    std::vector<int> m_QueueRoundRobinIndex; // TODO: lock this?
    std::vector<std::vector<std::unique_ptr<std::mutex>>> m_QueueMutexes;

    int FindQueueFamilyIndex(VulkanQueueType type);
};

} // namespace gore
