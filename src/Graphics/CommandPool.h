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
    struct Pool
    {
        const Device* m_Device;
        vk::raii::CommandPool commandPool;
        std::vector<vk::raii::CommandBuffer> commandBuffers;
        int currentPoolIndex;

        Pool();
        Pool(const Device& device, vk::raii::CommandPool commandPool);
        Pool(Pool&& other) noexcept;
        ~Pool();
    };

    static thread_local std::map<const CommandPool*, std::map<uint32_t, Pool>> s_CommandPools;

    const Device* m_Device;
    uint32_t m_QueueFamilyIndex;

    Pool& GetPool(uint32_t swapchainImageIndex);
};

} // namespace gore::gfx
