#include "RenderContextHelper.h"

#include <array>

namespace gore::gfx::VulkanHelper
{
inline vk::StencilOpState GetVkStencilOpState(const StencilOpState& state)
{
    vk::StencilOpState opState;
    opState.failOp      = static_cast<vk::StencilOp>(state.failOp);
    opState.passOp      = static_cast<vk::StencilOp>(state.passOp);
    opState.depthFailOp = static_cast<vk::StencilOp>(state.depthFailOp);
    opState.compareOp   = static_cast<vk::CompareOp>(state.compareOp);
    opState.compareMask = ~0;
    opState.writeMask   = ~0;
    opState.reference   = ~0;

    return opState;
}

vk::Format GetVkFormat(GraphicsFormat format)
{
    switch (format)
    {
        case GraphicsFormat::R11G11B10_UNORM:
            return vk::Format::eB10G11R11UfloatPack32;
        case GraphicsFormat::RGB32_FLOAT:
            return vk::Format::eR32G32B32Sfloat;
        case GraphicsFormat::BGRA8_SRGB:
            return vk::Format::eB8G8R8A8Srgb;
        case GraphicsFormat::RGB8_SRGB:
            return vk::Format::eR8G8B8Srgb;
        case GraphicsFormat::RGB8_UNORM:
            return vk::Format::eR8G8B8Unorm;
        case GraphicsFormat::RGBA8_SRGB:
            return vk::Format::eR8G8B8A8Srgb;
        case GraphicsFormat::RGBA8_UNORM:
            return vk::Format::eR8G8B8A8Unorm;
        case GraphicsFormat::RGB16_FLOAT:
            return vk::Format::eR16G16B16Sfloat;
        case GraphicsFormat::RG32_FLOAT:
            return vk::Format::eR32G32Sfloat;
        case GraphicsFormat::RG16_FLOAT:
            return vk::Format::eR16G16Sfloat;
        case GraphicsFormat::D32_FLOAT:
            return vk::Format::eD32Sfloat;
        case GraphicsFormat::D32_FLOAT_S8_UINT:
            return vk::Format::eD32SfloatS8Uint;
        case GraphicsFormat::Undefined:
            return vk::Format::eUndefined;
        default:
            std::runtime_error("Unknown format");
            return vk::Format::eUndefined;
    }
}

std::vector<VkFormat> GetVkFormats(const std::vector<GraphicsFormat>& formats)
{
    std::vector<VkFormat> vkFormats;
    vkFormats.reserve(formats.size());

    for (auto format : formats)
    {
        vkFormats.push_back(static_cast<VkFormat>(GetVkFormat(format)));
    }
    return vkFormats;
}

VkBufferCreateInfo GetVkBufferCreateInfo(BufferDesc& desc)
{
    VkBufferUsageFlags flags = 0;
    // TODO: I don't like switch
    switch (desc.usage)
    {
        case BufferUsage::Vertex:
            flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            break;
        case BufferUsage::Index:
            flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
            break;
        case BufferUsage::Uniform:
            flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            break;
        case BufferUsage::Storage:
            flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
            break;
        default:
            break;
    }

    VkBufferCreateInfo bufferInfo = {
        .sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size        = desc.byteSize,
        .usage       = flags | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    return bufferInfo;
}

// TODO: Finish this
VmaAllocationCreateInfo GetVmaAllocationCreateInfo(BufferDesc& desc)
{
    VmaMemoryUsage memUsage = VMA_MEMORY_USAGE_AUTO;

    // TODO: I don't like switch
    switch (desc.memUsage)
    {
        case MemoryUsage::GPU:
            memUsage = VMA_MEMORY_USAGE_GPU_ONLY;
            break;
        case MemoryUsage::CPU:
            memUsage = VMA_MEMORY_USAGE_CPU_ONLY;
            break;
        case MemoryUsage::CPU_TO_GPU:
            memUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
            break;
        case MemoryUsage::GPU_TO_CPU:
            memUsage = VMA_MEMORY_USAGE_GPU_TO_CPU;
            break;
        default:
            break;
    }

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage                   = memUsage;

    return allocInfo;
}


std::pair<std::vector<vk::VertexInputAttributeDescription>, std::vector<vk::VertexInputBindingDescription>> GetVkVertexInputState(const std::vector<VertexBufferBinding>& vertexBufferBindings)
{
    using namespace std;

    const int bindingSize = vertexBufferBindings.size();
    int attributeSize     = 0;
    for (auto& binding : vertexBufferBindings)
    {
        attributeSize += binding.attributes.size();
    }

    vector<vk::VertexInputAttributeDescription> attributeDescriptions(attributeSize);
    vector<vk::VertexInputBindingDescription> bindingDescriptions(bindingSize);

    // TODO: Parrallelize this
    for (int bindingIndex = 0; bindingIndex < bindingSize; bindingIndex++)
    {
        bindingDescriptions[bindingIndex].binding   = bindingIndex;
        bindingDescriptions[bindingIndex].stride    = vertexBufferBindings[bindingIndex].byteStride;
        bindingDescriptions[bindingIndex].inputRate = vk::VertexInputRate::eVertex;

        for (int attributeIndex = 0; attributeIndex < vertexBufferBindings[bindingIndex].attributes.size(); attributeIndex++)
        {
            attributeDescriptions[bindingIndex * attributeIndex + attributeIndex].binding  = bindingIndex;
            attributeDescriptions[bindingIndex * attributeIndex + attributeIndex].location = attributeIndex;
            attributeDescriptions[bindingIndex * attributeIndex + attributeIndex].format   = GetVkFormat(vertexBufferBindings[bindingIndex].attributes[attributeIndex].format);
            attributeDescriptions[bindingIndex * attributeIndex + attributeIndex].offset   = vertexBufferBindings[bindingIndex].attributes[attributeIndex].byteOffset;
        }
    }

    return {attributeDescriptions, bindingDescriptions};
}

vk::PipelineInputAssemblyStateCreateInfo GetVkInputAssemblyState(const GraphicsPipelineDesc& desc)
{
    auto& assemblyState = desc.assemblyState;

    return vk::PipelineInputAssemblyStateCreateInfo({}, GetVkPrimitiveTopology(assemblyState.topology), assemblyState.primitiveRestartEnable);
}

vk::PipelineViewportStateCreateInfo GetVkViewportState(const GraphicsPipelineDesc& desc)
{
    int viewPortCount   = desc.viewPortState.count;
    ViewPort* viewPorts = desc.viewPortState.viewPorts;

    int scissorCount = desc.scissorState.count;
    Rect* scissors   = desc.scissorState.scissors;

    return vk::PipelineViewportStateCreateInfo(
        {},
        viewPortCount,
        reinterpret_cast<vk::Viewport*>(viewPorts),
        scissorCount,
        reinterpret_cast<vk::Rect2D*>(scissors));
}

vk::PipelineRasterizationStateCreateInfo GetVkRasterizeState(const GraphicsPipelineDesc& desc)
{
    auto& rasterizeState = desc.rasterizeState;

    return vk::PipelineRasterizationStateCreateInfo(
        {},
        rasterizeState.depthClamp,
        rasterizeState.rasterizerDiscard,
        GetVkPolygonMode(rasterizeState.polygonMode),
        GetVkCullMode(rasterizeState.cullMode),
        GetVkFrontFace(rasterizeState.frontCounterClockwise),
        rasterizeState.depthBiasEnable,
        0.0f,
        1.0f,
        1.0f,
        1.0f);
}

vk::PipelineMultisampleStateCreateInfo GetVkMultisampleState(const GraphicsPipelineDesc& desc)
{
    auto& multisampleState = desc.multisampleState;

    vk::SampleCountFlagBits sampleCount = vk::SampleCountFlagBits::e1;
    switch (multisampleState.sampleCount)
    {
        case MultiSampleCount::One:
            sampleCount = vk::SampleCountFlagBits::e1;
            break;
        case MultiSampleCount::Two:
            sampleCount = vk::SampleCountFlagBits::e2;
            break;
        case MultiSampleCount::Four:
            sampleCount = vk::SampleCountFlagBits::e4;
            break;
        case MultiSampleCount::Eight:
            sampleCount = vk::SampleCountFlagBits::e8;
            break;
        default:
            break;
    }

    return vk::PipelineMultisampleStateCreateInfo(
        {},
        sampleCount,
        multisampleState.sampleShadingEnable,
        multisampleState.minSampleShading,
        &multisampleState.sampleMask,
        multisampleState.alphaToCoverageEnable,
        multisampleState.alphaToOneEnable);
}

vk::PipelineDepthStencilStateCreateInfo GetVkDepthStencilState(const GraphicsPipelineDesc& desc)
{
    auto& depthStencilState = desc.depthStencilState;

    return vk::PipelineDepthStencilStateCreateInfo(
        {},
        depthStencilState.depthTestEnable,
        depthStencilState.depthWriteEnable,
        GetVkCompareOp(depthStencilState.depthTestOp),
        depthStencilState.depthBoundsTestEnable,
        depthStencilState.stencilTestEnable,
        GetVkStencilOpState(depthStencilState.front),
        GetVkStencilOpState(depthStencilState.back),
        depthStencilState.minDepthBounds,
        depthStencilState.maxDepthBounds);
}

vk::PipelineColorBlendStateCreateInfo GetVkColorBlendState(const GraphicsPipelineDesc& desc)
{
    auto& colorBlendState = desc.blendState;

    return vk::PipelineColorBlendStateCreateInfo(
        {},
        colorBlendState.enable,
        GetVkLogicOp(colorBlendState.logicOp),
        colorBlendState.attachments.size(),
        reinterpret_cast<const vk::PipelineColorBlendAttachmentState*>(colorBlendState.attachments.data()),
        {1.0f, 1.0f, 1.0f, 1.0f});
}

void VulkanHelper::ImageLayoutTransition(vk::raii::CommandBuffer& commandBuffer, vk::raii::Image& image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::ImageSubresourceRange subResourceRange)
{


    std::vector<vk::ImageMemoryBarrier> barriers;
    barriers.push_back(vk::ImageMemoryBarrier(
        {},
        {},
        oldLayout,
        newLayout,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        *image,
        subResourceRange));
}

} // namespace gore::VulkanHelper