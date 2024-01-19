#pragma once

#include "Graphics/Vulkan/VulkanIncludes.h"
#include "Graphics/Vulkan/VulkanExtensions.h"

namespace gore::gfx
{
class VulkanBuffer
{
public:
    VulkanBuffer(vk::raii::Buffer&& buffer, VmaAllocation&& allocation, VmaAllocationInfo&& allocationInfo);

private:
    vk::raii::Buffer m_VkBuffer;
    VmaAllocation m_VmaAllocation;
    VmaAllocationInfo m_VmaAllocationInfo;
};
} // namespace gore::gfx