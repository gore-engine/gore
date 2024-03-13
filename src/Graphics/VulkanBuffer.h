#pragma once

#include "Graphics/Vulkan/VulkanIncludes.h"
#include "Graphics/Vulkan/VulkanExtensions.h"

namespace gore::gfx
{
struct VulkanBuffer
{
    VmaAllocator vmaAllocator;
    VkBuffer vkBuffer; // TODO: better name!
    VkDeviceAddress vkDeviceAddress;
    VmaAllocation vmaAllocation;
    VmaAllocationInfo vmaAllocationInfo;
};

void ClearVulkanBuffer(VmaAllocator allocator, VkBuffer buffer, VmaAllocation allocation)
{
    vmaDestroyBuffer(allocator, buffer, allocation);
}

bool IsMappableVulkanBuffer(const VulkanBuffer& buffer)
{
    VkMemoryPropertyFlags memPropFlags;
    vmaGetAllocationMemoryProperties(buffer.vmaAllocator, buffer.vmaAllocation, &memPropFlags);
    return memPropFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
}

void* MapVulkanBuffer(const VulkanBuffer& buffer)
{
    if (IsMappableVulkanBuffer(buffer))
    {
        assert(buffer.vmaAllocationInfo.pMappedData != nullptr);
        return buffer.vmaAllocationInfo.pMappedData;
    }

    void* data;
    vmaMapMemory(buffer.vmaAllocator, buffer.vmaAllocation, &data);
    return data;
}

void UnmapVulkanBuffer(const VulkanBuffer& buffer)
{
    vmaUnmapMemory(buffer.vmaAllocator, buffer.vmaAllocation);
}

void FlushVulkanBuffer(const VulkanBuffer& buffer, const uint32_t size = 0)
{
    vmaFlushAllocation(buffer.vmaAllocator, buffer.vmaAllocation, 0, size);
}

// TODO: Can be replaced by vma 3.1.0 vmaCopyAllocationToMemory
void SetBufferData(const VulkanBuffer& buffer, const uint8_t* data, const uint32_t size, const uint32_t offset = 0)
{
    bool isMappable = IsMappableVulkanBuffer(buffer);
    if (isMappable == false)
    {
        LOG_STREAM(ERROR, "VulkanBuffer", "SetBufferData: Buffer is not mappable!");
        return;
    }

    if (buffer.vmaAllocationInfo.pMappedData != nullptr)
    {
        uint8_t* mappedData = reinterpret_cast<uint8_t*>(buffer.vmaAllocationInfo.pMappedData);
        assert(mappedData != nullptr);
        memcpy(mappedData + offset, data, size);
    }
    else
    {
        uint8_t* mappedData;
        vmaMapMemory(buffer.vmaAllocator, buffer.vmaAllocation, reinterpret_cast<void**>(&mappedData));
        memcpy(mappedData + offset, data, size);
        vmaUnmapMemory(buffer.vmaAllocator, buffer.vmaAllocation);
    }
}

void SetBufferData(const VulkanBuffer& buffer, const std::vector<uint8_t>& data, const uint32_t offset = 0)
{
    SetBufferData(buffer, data.data(), static_cast<uint32_t>(data.size()), offset);
}

} // namespace gore::gfx