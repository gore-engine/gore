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

/// What kind of vertex input state we need to create a pipeline?
/// To simplify the process, we will always return only one verterInputBindingDescription and one vertexInputAttributeDescription array.
/// Maybe, multiple bindings and attributes are better for mobile devices, but for now, we will keep it simple.
std::pair<std::vector<vk::VertexInputAttributeDescription>, std::vector<vk::VertexInputBindingDescription>> GetVkVertexInputState(const std::vector<VertexBufferBinding>& vertexBufferBindings);
vk::PipelineInputAssemblyStateCreateInfo GetVkInputAssemblyState(const GraphicsPipelineDesc& desc);
vk::PipelineViewportStateCreateInfo GetVkViewportState(const GraphicsPipelineDesc& desc);
vk::PipelineRasterizationStateCreateInfo GetVkRasterizeState(const GraphicsPipelineDesc& desc);
} // namespace gore::vulkanHelper
