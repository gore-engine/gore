#include "RenderContextHelper.h"

namespace gore::vulkanHelper
{
VkBufferCreateInfo GetVkBufferCreateInfo(BufferDesc& desc)
{
    VkBufferUsageFlags flags = 0;
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

    VmaAllocationCreateInfo allocInfo = {
        .flags          = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
        .usage          = memUsage,
        .requiredFlags  = 0,
        .preferredFlags = 0,
        .memoryTypeBits = 0,
        .pool           = VK_NULL_HANDLE,
        .pUserData      = nullptr,
    };

    return allocInfo;
}

} // namespace gore::vulkanHelper