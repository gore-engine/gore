#pragma once

#include "Prefix.h"

#include "Graphics/Vulkan/VulkanIncludes.h"
#include "GraphicsResourceDesc.h"
#include "GraphicsPipelineDesc.h"

#include <vector>
namespace gore::gfx::VulkanHelper
{
std::vector<VkFormat> GetVkFormats(const std::vector<GraphicsFormat>& formats);

inline VkImageType GetVkImageType(TextureType type)
{
    return static_cast<VkImageType>(type);
}

inline VkImageUsageFlags GetVkImageUsageFlags(TextureUsageBits usage)
{
    VkImageUsageFlags flag = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    
    if (usage & TextureUsageBits::Sampled)
    {
        flag |= VK_IMAGE_USAGE_SAMPLED_BIT;
    }

    if (usage & TextureUsageBits::Storage)
    {
        flag |= VK_IMAGE_USAGE_STORAGE_BIT;
    }

    if (usage & TextureUsageBits::RenderTarget)
    {
        flag |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }

    if (usage & TextureUsageBits::DepthStencil)
    {
        flag |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }

    return flag;
}

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

inline vk::PipelineColorBlendAttachmentState GetVkColorBlendAttachmentState(const ColorAttachmentBlendState& state)
{
    return vk::PipelineColorBlendAttachmentState(
        state.enable,
        GetVkBlendFactor(state.srcColorFactor),
        GetVkBlendFactor(state.dstColorFactor),
        GetVkBlendOp(state.colorBlendOp),
        GetVkBlendFactor(state.srcAlphaFactor),
        GetVkBlendFactor(state.dstAlphaFactor),
        GetVkBlendOp(state.alphaBlendOp),
        static_cast<vk::ColorComponentFlagBits>(state.colorWriteMask));
}


vk::PipelineStageFlags GetPipelineStageFlags(vk::ImageLayout layout)
{
    switch (layout)
    {
        case vk::ImageLayout::eUndefined:
            return vk::PipelineStageFlagBits::eTopOfPipe;
        case vk::ImageLayout::ePreinitialized:
            return vk::PipelineStageFlagBits::eHost;
        case vk::ImageLayout::eTransferSrcOptimal:
        case vk::ImageLayout::eTransferDstOptimal:
            return vk::PipelineStageFlagBits::eTransfer;
        case vk::ImageLayout::eColorAttachmentOptimal:
            return vk::PipelineStageFlagBits::eColorAttachmentOutput;
        case vk::ImageLayout::eDepthStencilAttachmentOptimal:
            return vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests;
        case vk::ImageLayout::eFragmentShadingRateAttachmentOptimalKHR:
            return vk::PipelineStageFlagBits::eFragmentShadingRateAttachmentKHR;
        case vk::ImageLayout::eShaderReadOnlyOptimal:
            return vk::PipelineStageFlagBits::eVertexShader | vk::PipelineStageFlagBits::eFragmentShader;
        case vk::ImageLayout::ePresentSrcKHR:
            return vk::PipelineStageFlagBits::eBottomOfPipe;
        case vk::ImageLayout::eGeneral:
            assert(false && "General layout is not supported yet");
            return vk::PipelineStageFlagBits::eTopOfPipe;
        default:
            assert(false && "Unknown layout");
            return vk::PipelineStageFlagBits::eTopOfPipe;    
    }
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

void ImageLayoutTransition(vk::raii::CommandBuffer& commandBuffer, vk::raii::Image& image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::ImageSubresourceRange subResourceRange);
} // namespace gore::VulkanHelper
