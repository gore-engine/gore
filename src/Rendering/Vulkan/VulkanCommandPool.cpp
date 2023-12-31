#include "Prefix.h"

#include "VulkanCommandPool.h"
#include "VulkanDevice.h"
#include "VulkanCommandBuffer.h"
#include "VulkanSynchronization.h"

namespace gore
{

thread_local std::map<uint32_t, std::unique_ptr<VulkanCommandPool>> VulkanCommandPool::s_CommandPools;

VulkanCommandPool::VulkanCommandPool(VulkanDevice* device, uint32_t queueFamilyIndex) :
    m_Device(device),
    m_CommandPool(VK_NULL_HANDLE),
    m_QueueFamilyIndex(queueFamilyIndex)
{
    // TODO: we can remove VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT when we have proper in-flight checking.
    // TODO: for now we are using exactly one command pool per thread per queue family,
    // TODO: which bans us from resetting the command pool instead of the command buffers, which is not ideal.
    VkCommandPoolCreateInfo poolInfo{
        .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext            = nullptr,
        .flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = m_QueueFamilyIndex,
    };

    VkResult res = m_Device->API.vkCreateCommandPool(m_Device->Get(), &poolInfo, nullptr, &m_CommandPool);
    VK_CHECK_RESULT(res);
}

VulkanCommandPool::~VulkanCommandPool()
{
    if (m_CommandPool != VK_NULL_HANDLE)
    {
        m_Device->API.vkDestroyCommandPool(m_Device->Get(), m_CommandPool, nullptr);
    }
}

void VulkanCommandPool::Reset()
{
    m_Device->API.vkResetCommandPool(m_Device->Get(), m_CommandPool, 0);
}

VulkanCommandPool* VulkanCommandPool::GetOrCreate(VulkanDevice* device, uint32_t queueFamilyIndex)
{
    auto it = s_CommandPools.find(queueFamilyIndex);
    if (it != s_CommandPools.end())
        return it->second.get();

    s_CommandPools.emplace(queueFamilyIndex, std::unique_ptr<VulkanCommandPool>(new VulkanCommandPool(device, queueFamilyIndex)));
    return s_CommandPools[queueFamilyIndex].get();
}

void VulkanCommandPool::ClearAll()
{
    s_CommandPools.clear();
}

}