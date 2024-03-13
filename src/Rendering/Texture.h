#pragma once

#include "Prefix.h"
#include "Handle.h"

#include "Graphics/Vulkan/VulkanIncludes.h"

namespace gore::gfx
{
struct Texture
{
    VkImageView srv        = VK_NULL_HANDLE;
    VkImageView* uav       = nullptr;
    VkImageView srvStencil = VK_NULL_HANDLE;

    VkImage image                       = VK_NULL_HANDLE;
    VkDeviceMemory memory               = VK_NULL_HANDLE;
    VkImageView imageView               = VK_NULL_HANDLE;
    VkDeviceAddress deviceAddress       = 0;
    VmaAllocation vmaAllocation         = VK_NULL_HANDLE;
    VmaAllocationInfo vmaAllocationInfo = {};
};

using TextureHandle = Handle<Texture>;
} // namespace gore::gfx