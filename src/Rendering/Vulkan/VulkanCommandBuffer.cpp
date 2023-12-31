#include "Prefix.h"

#include "VulkanCommandBuffer.h"
#include "VulkanCommandPool.h"
#include "VulkanDevice.h"

namespace gore
{

VulkanCommandBuffer::VulkanCommandBuffer(VulkanCommandPool* commandPool, bool isPrimary) :
    m_CommandPool(commandPool),
    m_IsPrimary(isPrimary)
{
    VulkanDevice* device = m_CommandPool->GetDevice();
    VkCommandBufferAllocateInfo allocInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = m_CommandPool->Get(),
        .level = m_IsPrimary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY,
        .commandBufferCount = 1,
    };

    VkResult res = device->API.vkAllocateCommandBuffers(device->Get(), &allocInfo, &m_CommandBuffer);
    VK_CHECK_RESULT(res);
}

VulkanCommandBuffer::~VulkanCommandBuffer()
{
    if (m_CommandBuffer != VK_NULL_HANDLE)
    {
        VulkanDevice* device = m_CommandPool->GetDevice();
        device->API.vkFreeCommandBuffers(device->Get(), m_CommandPool->Get(), 1, &m_CommandBuffer);
    }
}

void VulkanCommandBuffer::Begin()
{
    VulkanDevice* device = m_CommandPool->GetDevice();
    VkCommandBufferBeginInfo beginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, // TODO
    };

    VkResult res = device->API.vkBeginCommandBuffer(m_CommandBuffer, &beginInfo);
    VK_CHECK_RESULT(res);
}
void VulkanCommandBuffer::End()
{
    VulkanDevice* device = m_CommandPool->GetDevice();
    VkResult res = device->API.vkEndCommandBuffer(m_CommandBuffer);
    VK_CHECK_RESULT(res);
}

}