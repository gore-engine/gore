#pragma once

#include "Prefix.h"
#include "Handle.h"

#include "Graphics/Vulkan/VulkanIncludes.h"

namespace gore
{
    struct Texture
    {
        VkImageView srv;
        VkImageView* uav;
        VkImageView srvStencil;

        VkImage image;
        VkDeviceMemory memory;
        VkImageView imageView;
        VkDeviceAddress deviceAddress;
        VmaAllocation vmaAllocation;
    };

    using TextureHandle = Handle<Texture>;
}