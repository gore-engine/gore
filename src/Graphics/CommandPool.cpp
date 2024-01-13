#include "Prefix.h"

#include "CommandPool.h"
#include "Device.h"

#include <map>

namespace gore::gfx
{

thread_local std::map<const CommandPool*, std::map<uint32_t, CommandPool::Pool>> CommandPool::s_CommandPools;

CommandPool::CommandPool() :
    m_Device(nullptr),
    m_QueueFamilyIndex(0)
{
}

CommandPool::CommandPool(const Device& device, uint32_t queueFamilyIndex) :
    m_Device(&device),
    m_QueueFamilyIndex(queueFamilyIndex)
{
}

CommandPool::CommandPool(CommandPool&& other) noexcept :
    m_Device(other.m_Device),
    m_QueueFamilyIndex(other.m_QueueFamilyIndex)
{
    if (s_CommandPools.contains(&other))
    {
        s_CommandPools.emplace(this, std::move(s_CommandPools.at(&other)));
        s_CommandPools.erase(&other);
    }
}

CommandPool::~CommandPool()
{
    if (s_CommandPools.contains(this))
    {
        s_CommandPools.erase(this);
    }
}

CommandPool& CommandPool::operator=(CommandPool&& other) noexcept
{
    m_Device           = other.m_Device;
    m_QueueFamilyIndex = other.m_QueueFamilyIndex;

    if (s_CommandPools.contains(&other))
    {
        s_CommandPools.emplace(this, std::move(s_CommandPools.at(&other)));
        s_CommandPools.erase(&other);
    }

    return *this;
}

const vk::raii::CommandPool& CommandPool::Get(uint32_t swapchainImageIndex)
{
    return GetPool(swapchainImageIndex).commandPool;
}

CommandPool::Pool& CommandPool::GetPool(uint32_t swapchainImageIndex)
{
    if (!s_CommandPools.contains(this))
    {
        s_CommandPools.emplace(this, std::map<uint32_t, Pool>());
    }

    if (!s_CommandPools.at(this).contains(swapchainImageIndex))
    {
        vk::CommandPoolCreateInfo createInfo({}, m_QueueFamilyIndex);
        s_CommandPools.at(this).emplace(swapchainImageIndex, Pool(*m_Device, vk::raii::CommandPool(m_Device->Get(), createInfo)));
    }

    return s_CommandPools.at(this).at(swapchainImageIndex);
}

const vk::raii::CommandBuffer& CommandPool::GetCommandBuffer(uint32_t swapchainImageIndex)
{
    Pool& pool = GetPool(swapchainImageIndex);

    if (pool.currentPoolIndex >= pool.commandBuffers.size())
    {
        // TODO: secondary command buffers
        vk::CommandBufferAllocateInfo allocateInfo(*pool.commandPool, vk::CommandBufferLevel::ePrimary, 1);
        pool.commandBuffers.emplace_back(std::move(m_Device->Get().allocateCommandBuffers(allocateInfo)[0]));
    }

    return pool.commandBuffers[pool.currentPoolIndex++];
}

void CommandPool::Reset(uint32_t swapchainImageIndex)
{
    Pool& pool = GetPool(swapchainImageIndex);

    pool.commandPool.reset({});
    pool.currentPoolIndex = 0;
}

CommandPool::Pool::Pool() :
    m_Device(nullptr),
    commandPool(nullptr),
    commandBuffers(),
    currentPoolIndex(0)
{
}

CommandPool::Pool::Pool(const Device& device, vk::raii::CommandPool commandPool) :
    m_Device(&device),
    commandPool(std::move(commandPool)),
    commandBuffers(),
    currentPoolIndex(0)
{
}

CommandPool::Pool::Pool(CommandPool::Pool&& other) noexcept :
    m_Device(other.m_Device),
    commandPool(std::move(other.commandPool)),
    commandBuffers(std::move(other.commandBuffers)),
    currentPoolIndex(other.currentPoolIndex)
{
}

CommandPool::Pool::~Pool()
{
}

} // namespace gore::gfx
