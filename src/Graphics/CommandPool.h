#pragma once

#include "Graphics/Vulkan/VulkanIncludes.h"
#include "Graphics/Vulkan/VulkanExtensions.h"

#include <map>

namespace gore::gfx
{

class Device;

class CommandPool
{
public:
    CommandPool();
    CommandPool(const Device& device, uint32_t queueFamilyIndex);
    CommandPool(CommandPool&& other) noexcept;
    ~CommandPool();

    CommandPool& operator=(CommandPool&& other) noexcept;

    [[nodiscard]] const vk::raii::CommandPool& Get(uint32_t swapchainImageIndex);

    [[nodiscard]] const vk::raii::CommandBuffer& GetCommandBuffer(uint32_t swapchainImageIndex);
    void Reset(uint32_t swapchainImageIndex);

private:
    struct CommandPoolEntry
    {
        const Device* m_Device;
        vk::raii::CommandPool commandPool;
        std::vector<vk::raii::CommandBuffer> commandBuffers;
        int currentPoolIndex;

        CommandPoolEntry();
        CommandPoolEntry(const Device& device, vk::raii::CommandPool commandPool);
        CommandPoolEntry(CommandPoolEntry&& other) noexcept;
        ~CommandPoolEntry();
    };

    static thread_local std::map<const CommandPool*, std::map<uint32_t, CommandPoolEntry>> s_PoolEntries;

    const Device* m_Device;
    uint32_t m_QueueFamilyIndex;

    CommandPoolEntry& GetPoolEntry(uint32_t swapchainImageIndex);
};

} // namespace gore::gfx
