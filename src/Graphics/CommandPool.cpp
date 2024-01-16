#include "Prefix.h"

#include "CommandPool.h"
#include "Device.h"

#include <map>

namespace gore::gfx
{

thread_local std::map<const CommandPool*, std::map<uint32_t, CommandPool::CommandPoolEntry>> CommandPool::s_PoolEntries;

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
    if (s_PoolEntries.contains(&other))
    {
        s_PoolEntries.emplace(this, std::move(s_PoolEntries.at(&other)));
        s_PoolEntries.erase(&other);
    }
}

CommandPool::~CommandPool()
{
    if (s_PoolEntries.contains(this))
    {
        s_PoolEntries.erase(this);
    }
}

CommandPool& CommandPool::operator=(CommandPool&& other) noexcept
{
    m_Device           = other.m_Device;
    m_QueueFamilyIndex = other.m_QueueFamilyIndex;

    if (s_PoolEntries.contains(&other))
    {
        s_PoolEntries.emplace(this, std::move(s_PoolEntries.at(&other)));
        s_PoolEntries.erase(&other);
    }

    return *this;
}

const vk::raii::CommandPool& CommandPool::Get(uint32_t swapchainImageIndex)
{
    return GetPoolEntry(swapchainImageIndex).commandPool;
}

CommandPool::CommandPoolEntry& CommandPool::GetPoolEntry(uint32_t swapchainImageIndex)
{
    if (!s_PoolEntries.contains(this))
    {
        s_PoolEntries.emplace(this, std::map<uint32_t, CommandPoolEntry>());
    }

    if (!s_PoolEntries.at(this).contains(swapchainImageIndex))
    {
        vk::CommandPoolCreateInfo createInfo({}, m_QueueFamilyIndex);
        s_PoolEntries.at(this).emplace(swapchainImageIndex, CommandPoolEntry(*m_Device, vk::raii::CommandPool(m_Device->Get(), createInfo)));
    }

    return s_PoolEntries.at(this).at(swapchainImageIndex);
}

const vk::raii::CommandBuffer& CommandPool::GetCommandBuffer(uint32_t swapchainImageIndex)
{
    CommandPoolEntry& pool = GetPoolEntry(swapchainImageIndex);

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
    CommandPoolEntry& pool = GetPoolEntry(swapchainImageIndex);

    pool.commandPool.reset({});
    pool.currentPoolIndex = 0;
}

CommandPool::CommandPoolEntry::CommandPoolEntry() :
    m_Device(nullptr),
    commandPool(nullptr),
    commandBuffers(),
    currentPoolIndex(0)
{
}

CommandPool::CommandPoolEntry::CommandPoolEntry(const Device& device, vk::raii::CommandPool commandPool) :
    m_Device(&device),
    commandPool(std::move(commandPool)),
    commandBuffers(),
    currentPoolIndex(0)
{
}

CommandPool::CommandPoolEntry::CommandPoolEntry(CommandPool::CommandPoolEntry&& other) noexcept :
    m_Device(other.m_Device),
    commandPool(std::move(other.commandPool)),
    commandBuffers(std::move(other.commandBuffers)),
    currentPoolIndex(other.currentPoolIndex)
{
}

CommandPool::CommandPoolEntry::~CommandPoolEntry()
{
}

} // namespace gore::gfx
