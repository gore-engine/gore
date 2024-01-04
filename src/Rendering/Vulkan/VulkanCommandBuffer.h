#pragma once

#include "VulkanIncludes.h"
#include "VulkanBarrier.h"

#include <vector>

namespace gore
{

class VulkanCommandPool;

class VulkanCommandBuffer
{
public:
    explicit VulkanCommandBuffer(VulkanCommandPool* commandPool, bool isPrimary = true);
    ~VulkanCommandBuffer();

    [[nodiscard]] VkCommandBuffer Get() const { return m_CommandBuffer; }
    [[nodiscard]] VulkanCommandPool* GetCommandPool() const { return m_CommandPool; }

    void Begin();
    void End();

    void Barrier(const std::vector<VulkanResourceBarrier>& barriers, VulkanQueueType currentQueueType, VulkanQueueType newQueueType);

private:
    VulkanCommandPool* m_CommandPool;

    VkCommandBuffer m_CommandBuffer;
    bool m_IsPrimary;
};

} // namespace gore
