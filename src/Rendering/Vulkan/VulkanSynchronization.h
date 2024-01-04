#pragma once

#include "VulkanIncludes.h"

namespace gore
{

class VulkanDevice;

class VulkanSemaphore
{
public:
    explicit VulkanSemaphore(VulkanDevice* device);
    ~VulkanSemaphore();

    [[nodiscard]] VkSemaphore Get() const { return m_Semaphore; }

private:
    VulkanDevice* m_Device;

    VkSemaphore m_Semaphore;
};

class VulkanFence
{
public:
    explicit VulkanFence(VulkanDevice* device, bool signaled = false);
    ~VulkanFence();

    [[nodiscard]] VkFence Get() const { return m_Fence; }

    void Reset();
    void Wait();

private:
    VulkanDevice* m_Device;

    VkFence m_Fence;
};

}
