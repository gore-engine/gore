#include "RenderContextHelper.h"

namespace gore::vulkanHelper
{
vk::Format GetVkFormat(GraphicsFormat format)
{
    switch (format)
    {
        case GraphicsFormat::R11G11B10_UNORM:
            return vk::Format::eB10G11R11UfloatPack32;
        case GraphicsFormat::RGB32_FLOAT:
            return vk::Format::eR32G32B32Sfloat;
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
        default:
            std::runtime_error("Unknown format");
            return;
    }
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

    return std::pair<std::vector<vk::VertexInputAttributeDescription>, std::vector<vk::VertexInputBindingDescription>>();
}

} // namespace gore::vulkanHelper