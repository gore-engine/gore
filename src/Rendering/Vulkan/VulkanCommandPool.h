#pragma once

#include "VulkanIncludes.h"

#include <map>
#include <memory>

namespace gore
{

class VulkanDevice;

class VulkanCommandPool
{
public:
    void Reset();

    [[nodiscard]] VkCommandPool Get() const { return m_CommandPool; }
    [[nodiscard]] uint32_t GetQueueFamilyIndex() const { return m_QueueFamilyIndex; }
    [[nodiscard]] VulkanDevice* GetDevice() const { return m_Device; }

    [[nodiscard]] static VulkanCommandPool* GetOrCreate(VulkanDevice* device, uint32_t queueFamilyIndex);
    static void ClearAll();

private:
    friend std::unique_ptr<VulkanCommandPool>::deleter_type;
    explicit VulkanCommandPool(VulkanDevice* device, uint32_t queueFamilyIndex);
    ~VulkanCommandPool();

    static thread_local std::map<uint32_t, std::unique_ptr<VulkanCommandPool>> s_CommandPools;

    VkCommandPool m_CommandPool;
    uint32_t m_QueueFamilyIndex;

    VulkanDevice* m_Device;
};

} // namespace gore
