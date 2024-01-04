#include "Prefix.h"

#include "VulkanSynchronization.h"
#include "VulkanDevice.h"

namespace gore
{

VulkanSemaphore::VulkanSemaphore(VulkanDevice* device):
    m_Device(device),
    m_Semaphore(VK_NULL_HANDLE)
{
    VkSemaphoreCreateInfo semaphoreCreateInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = VK_NULL_HANDLE, // TODO: Add timeline semaphore support
        .flags = 0,
    };

    VkResult res = m_Device->API.vkCreateSemaphore(m_Device->Get(), &semaphoreCreateInfo, VK_NULL_HANDLE, &m_Semaphore);
    VK_CHECK_RESULT(res);
}

VulkanSemaphore::~VulkanSemaphore()
{
    if (m_Semaphore != VK_NULL_HANDLE)
    {
        m_Device->API.vkDestroySemaphore(m_Device->Get(), m_Semaphore, VK_NULL_HANDLE);
    }
}

VulkanFence::VulkanFence(VulkanDevice* device, bool signaled) :
    m_Device(device),
    m_Fence(VK_NULL_HANDLE)
{
    VkFenceCreateInfo fenceCreateInfo{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0u,
    };

    VkResult res = device->API.vkCreateFence(device->Get(), &fenceCreateInfo, VK_NULL_HANDLE, &m_Fence);
    VK_CHECK_RESULT(res);
}

VulkanFence::~VulkanFence()
{
    if (m_Fence != VK_NULL_HANDLE)
    {
        m_Device->API.vkDestroyFence(m_Device->Get(), m_Fence, VK_NULL_HANDLE);
    }
}

void VulkanFence::Reset()
{
    m_Device->API.vkResetFences(m_Device->Get(), 1, &m_Fence);
}

// TODO: Add timeout parameter
void VulkanFence::Wait()
{
    m_Device->API.vkWaitForFences(m_Device->Get(), 1, &m_Fence, VK_TRUE, UINT64_MAX);
}

}