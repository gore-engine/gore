#pragma once

#include "Prefix.h"

#include "Graphics/Vulkan/VulkanIncludes.h"
#include "GraphicsResourceDesc.h"
#include "GraphicsPipelineDesc.h"

#include <vector>
namespace gore::vulkanHelper
{
    vk::Format GetVkFormat(GraphicsFormat format);

    VkBufferCreateInfo GetVkBufferCreateInfo(BufferDesc& desc);
    VmaAllocationCreateInfo GetVmaAllocationCreateInfo(BufferDesc& desc);

    std::pair<std::vector<vk::VertexInputAttributeDescription>, std::vector<vk::VertexInputBindingDescription>> GetVkVertexInputState(const std::vector<VertexBufferBinding>& vertexBufferBindings);
} // namespace gore