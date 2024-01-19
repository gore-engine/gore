#include "VulkanBuffer.h"

namespace gore::gfx
{
VulkanBuffer::VulkanBuffer(vk::raii::Buffer&& buffer, VmaAllocation&& allocation, VmaAllocationInfo&& allocationInfo) :
    m_VkBuffer(std::move(buffer)),
    m_VmaAllocation(std::move(allocation)),
    m_VmaAllocationInfo(std::move(allocationInfo))
{
}
} // namespace gore::gfx