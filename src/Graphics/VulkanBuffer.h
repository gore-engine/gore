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

void ClearVulkanBuffer(VmaAllocator allocator, VkBuffer buffer, VmaAllocation allocation);

bool IsMappableVulkanBuffer(const VulkanBuffer& buffer);

void* MapVulkanBuffer(const VulkanBuffer& buffer);

void UnmapVulkanBuffer(const VulkanBuffer& buffer);

void FlushVulkanBuffer(const VulkanBuffer& buffer, const uint32_t size);

void SetBufferData(const VulkanBuffer& buffer, const uint8_t* data, const uint32_t size, const uint32_t offset);

void SetBufferData(const VulkanBuffer& buffer, const std::vector<uint8_t>& data, const uint32_t offset);

} // namespace gore::gfx