#pragma once

#include "VulkanIncludes.h"

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

private:
    VulkanCommandPool* m_CommandPool;

    VkCommandBuffer m_CommandBuffer;
    bool m_IsPrimary;
};

} // namespace gore
