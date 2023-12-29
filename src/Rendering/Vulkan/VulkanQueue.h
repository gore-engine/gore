#pragma once

#include "VulkanIncludes.h"

namespace gore
{

class VulkanDevice;

enum class VulkanQueueType
{
    Graphics,
    Compute,
    Transfer,
    SparseBinding,
#ifdef VK_KHR_video_decode_queue
    VideoDecode,
#endif
#if defined(VK_KHR_video_encode_queue) && defined(VK_ENABLE_BETA_EXTENSIONS)
    VideoEncode,
#endif
    Present
};

class VulkanQueue
{
public:
    VulkanQueue(VulkanDevice* device, uint32_t familyIndex, uint32_t queueIndex, VkQueue queue, VkQueueFlags flags, bool presentable);
    ~VulkanQueue();

    [[nodiscard]] VkQueue Get() const { return m_Queue; }
    [[nodiscard]] uint32_t GetFamilyIndex() const { return m_FamilyIndex; }
    [[nodiscard]] uint32_t GetQueueIndex() const { return m_QueueIndex; }
    [[nodiscard]] bool IsCapableOf(VulkanQueueType type) const;

    [[nodiscard]] static bool IsCapableOf(VkQueueFlags flags, bool presentable, VulkanQueueType type);
    [[nodiscard]] static int QueueFlagBitCount(VkQueueFlags flags, bool presentable);

private:
    VulkanDevice* m_Device;

    VkQueue m_Queue;
    uint32_t m_FamilyIndex;
    uint32_t m_QueueIndex;
    VkQueueFlags m_Flags;
    bool m_Presentable;

    [[nodiscard]] std::mutex& GetMutex() const;
};

} // namespace gore
