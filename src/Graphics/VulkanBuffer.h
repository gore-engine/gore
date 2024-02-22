#pragma once

#include "Graphics/Vulkan/VulkanIncludes.h"
#include "Graphics/Vulkan/VulkanExtensions.h"

namespace gore::gfx
{
struct VulkanBuffer
{
    void Clear()
    {
        vmaDestroyBuffer(vmaAllocator, vkBuffer, vmaAllocation);
    }

    VmaAllocator vmaAllocator;
    VkBuffer vkBuffer; // TODO: better name!
    VkDeviceAddress vkDeviceAddress;
    VmaAllocation vmaAllocation;
    VmaAllocationInfo vmaAllocationInfo;
};
} // namespace gore::gfx