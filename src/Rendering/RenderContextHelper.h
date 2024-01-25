#pragma once

#include "Prefix.h"

#include "Graphics/Vulkan/VulkanIncludes.h"
#include "GraphicsResourceDesc.h"

namespace gore::vulkanHelper
{
    VkBufferCreateInfo GetVkBufferCreateInfo(BufferDesc& desc);
    VmaAllocationCreateInfo GetVmaAllocationCreateInfo(BufferDesc& desc);
} // namespace gore