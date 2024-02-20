#pragma once

#include "Prefix.h"

#include "Graphics/Vulkan/VulkanIncludes.h"
#include "GraphicsResourceDesc.h"
#include "GraphicsPipelineDesc.h"

#include <vector>
namespace gore::VulkanHelper
{
inline vk::CompareOp GetVkCompareOp(CompareOp op)
{
    return static_cast<vk::CompareOp>(op);
}

inline vk::PrimitiveTopology GetVkPrimitiveTopology(TopologyType topology)
{
    switch (topology)
    {
        case TopologyType::Point:
            return vk::PrimitiveTopology::ePointList;
        case TopologyType::Line:
            return vk::PrimitiveTopology::eLineList;
        case TopologyType::TriangleList:
            return vk::PrimitiveTopology::eTriangleList;
        default:
            return vk::PrimitiveTopology::eTriangleList;
    }
}

inline vk::PolygonMode GetVkPolygonMode(PolygonMode mode)
{
    switch (mode)
    {
        case PolygonMode::Fill:
            return vk::PolygonMode::eFill;
        case PolygonMode::Line:
            return vk::PolygonMode::eLine;
        case PolygonMode::Point:
            return vk::PolygonMode::ePoint;
        default:
            return vk::PolygonMode::eFill;
    }
}

inline vk::CullModeFlags GetVkCullMode(CullMode mode)
{
    switch (mode)
    {
        case CullMode::None:
            return vk::CullModeFlagBits::eNone;
        case CullMode::Front:
            return vk::CullModeFlagBits::eFront;
        case CullMode::Back:
            return vk::CullModeFlagBits::eBack;
        default:
            return vk::CullModeFlagBits::eNone;
    }
}

inline vk::FrontFace GetVkFrontFace(bool frontCounterClockwise)
{
    return frontCounterClockwise ? vk::FrontFace::eCounterClockwise : vk::FrontFace::eClockwise;
}

inline vk::BlendOp GetVkBlendOp(BlendOp op)
{
    return static_cast<vk::BlendOp>(op);
}

inline vk::BlendFactor GetVkBlendFactor(BlendFactor factor)
{
    return static_cast<vk::BlendFactor>(factor);
}

inline vk::LogicOp GetVkLogicOp(LogicOp op)
{
    return static_cast<vk::LogicOp>(op);
}

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
vk::PipelineMultisampleStateCreateInfo GetVkMultisampleState(const GraphicsPipelineDesc& desc);
vk::PipelineDepthStencilStateCreateInfo GetVkDepthStencilState(const GraphicsPipelineDesc& desc);
vk::PipelineColorBlendStateCreateInfo GetVkColorBlendState(const GraphicsPipelineDesc& desc);
vk::PipelineDynamicStateCreateInfo GetVkDynamicState(const GraphicsPipelineDesc& desc);
} // namespace gore::VulkanHelper
