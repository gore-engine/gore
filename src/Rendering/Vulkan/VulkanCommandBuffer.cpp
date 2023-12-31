#include "Prefix.h"

#include "VulkanCommandBuffer.h"
#include "VulkanCommandPool.h"
#include "VulkanDevice.h"
#include "VulkanImage.h"

namespace gore
{

VulkanCommandBuffer::VulkanCommandBuffer(VulkanCommandPool* commandPool, bool isPrimary) :
    m_CommandPool(commandPool),
    m_IsPrimary(isPrimary)
{
    VulkanDevice* device = m_CommandPool->GetDevice();
    VkCommandBufferAllocateInfo allocInfo{
        .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool        = m_CommandPool->Get(),
        .level              = m_IsPrimary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY,
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
    VkResult res         = device->API.vkEndCommandBuffer(m_CommandBuffer);
    VK_CHECK_RESULT(res);
}

void VulkanCommandBuffer::Barrier(const std::vector<VulkanResourceBarrier>& barriers, VulkanQueueType queueType)
{
    VulkanDevice* device = m_CommandPool->GetDevice();

    std::vector<VkImageMemoryBarrier> imageBarriers;
    std::vector<VkBufferMemoryBarrier> bufferBarriers;

    VkAccessFlags srcAccessMask = 0;
    VkAccessFlags dstAccessMask = 0;

    for (const auto& barrier : barriers)
    {
        switch (barrier.type)
        {
            // case ResourceBarrierType::Buffer:
            // {
            //     VkBufferMemoryBarrier bufferBarrier{
            //         .sType               = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
            //         .pNext               = VK_NULL_HANDLE,
            //         .srcAccessMask       = AccessFlagsFromResourceState(barrier.currentState),
            //         .dstAccessMask       = AccessFlagsFromResourceState(barrier.newState),
            //         .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, // TODO
            //         .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, // TODO
            //         .buffer              = barrier.buffer->Get(),
            //         .offset              = 0,
            //         .size                = barrier.buffer->GetSize()
            //     };

            //     srcAccessMask |= bufferBarrier.srcAccessMask;
            //     dstAccessMask |= bufferBarrier.dstAccessMask;
            //     bufferBarriers.push_back(bufferBarrier);
            //     break;
            // }
            case ResourceBarrierType::Image:
            {
                VkImageMemoryBarrier imageBarrier{
                    .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                    .pNext               = VK_NULL_HANDLE,
                    .srcAccessMask       = AccessFlagsFromResourceState(barrier.currentState),
                    .dstAccessMask       = AccessFlagsFromResourceState(barrier.newState),
                    .oldLayout           = ImageLayoutFromResourceState(barrier.currentState),
                    .newLayout           = ImageLayoutFromResourceState(barrier.newState),
                    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, // TODO
                    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, // TODO
                    .image               = barrier.image->Get(),
                    .subresourceRange    = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}
                }; // TODO: subresource range

                srcAccessMask |= imageBarrier.srcAccessMask;
                dstAccessMask |= imageBarrier.dstAccessMask;
                imageBarriers.push_back(imageBarrier);
                break;
            }
            default:
                break;
        }
    }

    device->API.vkCmdPipelineBarrier(m_CommandBuffer,
                                     PipelineStageFlagsFromAccessFlags(srcAccessMask, queueType),
                                     PipelineStageFlagsFromAccessFlags(dstAccessMask, queueType),
                                     0,
                                     0,
                                     VK_NULL_HANDLE,
                                     static_cast<uint32_t>(bufferBarriers.size()),
                                     bufferBarriers.data(),
                                     static_cast<uint32_t>(imageBarriers.size()),
                                     imageBarriers.data());
}

} // namespace gore