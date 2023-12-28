#include "Prefix.h"

#include "VulkanQueue.h"

#include "Core/Log.h"

namespace gore
{

VulkanQueue::VulkanQueue(VulkanDevice* device, uint32_t familyIndex, uint32_t queueIndex, VkQueue queue, VkQueueFlags flags, bool presentable) :
    m_Device(device),
    m_Queue(queue),
    m_FamilyIndex(familyIndex),
    m_QueueIndex(queueIndex),
    m_Flags(flags),
    m_Presentable(presentable)
{
    LOG_STREAM(DEBUG) << "Created Vulkan Queue in family " << familyIndex << " with index " << queueIndex << std::endl;
}

VulkanQueue::~VulkanQueue()
{
}

bool VulkanQueue::IsCapableOf(VulkanQueueType type) const
{
    return IsCapableOf(m_Flags, m_Presentable, type);
}

bool VulkanQueue::IsCapableOf(VkQueueFlags flags, bool presentable, VulkanQueueType type)
{
    switch (type)
    {
        case VulkanQueueType::Graphics:
            return (flags & VK_QUEUE_GRAPHICS_BIT) != 0;
        case VulkanQueueType::Compute:
            return (flags & VK_QUEUE_COMPUTE_BIT) != 0;
        case VulkanQueueType::Transfer:
            return (flags & VK_QUEUE_TRANSFER_BIT) != 0;
        case VulkanQueueType::SparseBinding:
            return (flags & VK_QUEUE_SPARSE_BINDING_BIT) != 0;
#ifdef VK_KHR_video_decode_queue
        case VulkanQueueType::VideoDecode:
            return (flags & VK_QUEUE_VIDEO_DECODE_BIT_KHR) != 0;
#endif
#if defined(VK_KHR_video_encode_queue) && defined(VK_ENABLE_BETA_EXTENSIONS)
        case VulkanQueueType::VideoEncode:
            return (flags & VK_QUEUE_VIDEO_ENCODE_BIT_KHR) != 0;
#endif
        case VulkanQueueType::Present:
            return presentable;
        default:
            return false;
    }
}

int VulkanQueue::QueueFlagBitCount(VkQueueFlags flags, bool presentable)
{
    int count = 0;

    if (flags & VK_QUEUE_GRAPHICS_BIT)
        count++;
    if (flags & VK_QUEUE_COMPUTE_BIT)
        count++;
    if (flags & VK_QUEUE_TRANSFER_BIT)
        count++;
    if (flags & VK_QUEUE_SPARSE_BINDING_BIT)
        count++;
#ifdef VK_KHR_video_decode_queue
    if (flags & VK_QUEUE_VIDEO_DECODE_BIT_KHR)
        count++;
#endif
#if defined(VK_KHR_video_encode_queue) && defined(VK_ENABLE_BETA_EXTENSIONS)
    if (flags & VK_QUEUE_VIDEO_ENCODE_BIT_KHR)
        count++;
#endif
    if (presentable)
        count++;

    return count;
}

} // namespace gore