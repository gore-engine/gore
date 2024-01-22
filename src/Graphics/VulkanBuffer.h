#pragma once

#include "Graphics/Vulkan/VulkanIncludes.h"
#include "Graphics/Vulkan/VulkanExtensions.h"

namespace gore::gfx
{
struct VulkanBuffer
{
    VkBuffer vkBuffer;
    VmaAllocation vmaAllocation;
    VmaAllocationInfo vmaAllocationInfo;
};
} // namespace gore::gfx