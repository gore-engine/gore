#pragma once
#include "Prefix.h"

#include "Handle.h"
#include "Rendering/GraphicsResourcePrefix.h"

#include "Graphics/Vulkan/VulkanIncludes.h"
#include "Graphics/Vulkan/VulkanExtensions.h"

namespace gore::gfx
{
struct BufferDesc final
{
    const char* debugName = "Noname VertexBufferDesc";
    uint32_t byteSize     = 0;
    BufferUsage usage     = BufferUsage::Vertex;
    MemoryUsage memUsage  = MemoryUsage::GPU;
    const void* data      = nullptr;
};

struct Buffer
{
    VmaAllocator vmaAllocator           = VK_NULL_HANDLE;
    VkBuffer vkBuffer                   = VK_NULL_HANDLE; // TODO: better name!
    VkDeviceAddress vkDeviceAddress     = 0;
    VmaAllocation vmaAllocation         = VK_NULL_HANDLE;
    VmaAllocationInfo vmaAllocationInfo = {};
};

using BufferHandle = Handle<Buffer>;

void ClearVulkanBuffer(VmaAllocator allocator, VkBuffer buffer, VmaAllocation allocation);

bool IsMappableVulkanBuffer(const Buffer& buffer);

void* MapVulkanBuffer(const Buffer& buffer);

void UnmapVulkanBuffer(const Buffer& buffer);

void FlushVulkanBuffer(const Buffer& buffer, const uint32_t size);

void SetBufferData(const Buffer& buffer, const uint8_t* data, const uint32_t size, const uint32_t offset);

void SetBufferData(const Buffer& buffer, const std::vector<uint8_t>& data, const uint32_t offset);
} // namespace gore::gfx