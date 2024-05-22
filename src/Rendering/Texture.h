#pragma once

#include "Prefix.h"
#include "Handle.h"

#include "Graphics/Vulkan/VulkanIncludes.h"

namespace gore::gfx
{
struct Texture
{
    vk::ImageView srv        = VK_NULL_HANDLE;
    vk::ImageView* uav       = nullptr;
    vk::ImageView srvStencil = VK_NULL_HANDLE;

    VkImage image                       = VK_NULL_HANDLE;
    vk::DeviceMemory memory               = VK_NULL_HANDLE;
    vk::ImageView imageView               = VK_NULL_HANDLE;
    vk::DeviceAddress deviceAddress       = 0;
    VmaAllocation vmaAllocation         = VK_NULL_HANDLE;
    VmaAllocationInfo vmaAllocationInfo = {};
};

using TextureHandle = Handle<Texture>;

inline void DestroyVulkanTexture(VmaAllocator vmaAllocator, VkImage image, VmaAllocation vmaAllocation)
{
    vmaDestroyImage(vmaAllocator, image, vmaAllocation);
}

} // namespace gore::gfx