#include "RenderContextHelper.h"

namespace gore::vulkanHelper
{
VkBufferCreateInfo GetVkBufferCreateInfo(BufferDesc& desc)
{
    VkBufferUsageFlags flags = 0;
    // TODO: I don't like switch
    switch (desc.usage)
    {
        case BufferUsage::Vertex:
            // TODO: On mobile, we should use vertex bit?
            flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
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

} // namespace gore::vulkanHelper