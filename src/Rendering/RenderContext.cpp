#include "RenderContext.h"
#include "Graphics/Device.h"

namespace gore
{
RenderContext::RenderContext(const gfx::Device* device) :
    m_DevicePtr(device),
    m_ShaderModulePool()
{
}

RenderContext::~RenderContext()
{
    clear();
}

void RenderContext::clear()
{
    m_ShaderModulePool.clear();
}

ShaderModuleHandle RenderContext::createShaderModule(ShaderModuleDesc&& desc)
{
    return m_ShaderModulePool.create(
        std::move(desc),
        std::move(ShaderModule(
            m_DevicePtr->Get().createShaderModule(
                vk::ShaderModuleCreateInfo(
                    {},
                    desc.byteSize,
                    reinterpret_cast<const uint32_t*>(desc.byteCode))))));
}

const ShaderModuleDesc& RenderContext::getShaderModuleDesc(ShaderModuleHandle handle)
{
    return m_ShaderModulePool.getObjectDesc(handle);
}


const ShaderModule& RenderContext::getShaderModule(ShaderModuleHandle handle)
{
    return m_ShaderModulePool.getObject(handle);
}

void RenderContext::destroyShaderModule(ShaderModuleHandle handle)
{
    m_ShaderModulePool.destroy(handle);
}

BufferHandle RenderContext::CreateBuffer(BufferDesc&& desc)
{
    using namespace gfx;

    uint32_t byteSize = desc.byteSize;

    VkBufferCreateInfo bufferInfo = {
        .sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size        = byteSize,
        .usage       = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    VmaAllocationCreateInfo allocCreateInfo = {
        .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_CPU_TO_GPU,
    };

    VulkanBuffer buffer;

    VK_CHECK_RESULT(vmaCreateBuffer(m_DevicePtr->GetVmaAllocator(), &bufferInfo, &allocCreateInfo, &buffer.vkBuffer, &buffer.vmaAllocation, &buffer.vmaAllocationInfo));

    return m_BufferPool.create(
        std::move(desc),
        std::move(Buffer(std::move(buffer))));
}

const BufferDesc& RenderContext::GetBufferDesc(BufferHandle handle)
{
    return m_BufferPool.getObjectDesc(handle);
}

const Buffer& RenderContext::GetBuffer(BufferHandle handle)
{
    return m_BufferPool.getObject(handle);
}

void RenderContext::DestroyBuffer(BufferHandle handle)
{
    vmaDestroyBuffer(m_DevicePtr->GetVmaAllocator(), m_BufferPool.getObject(handle).vkBuffer.vkBuffer, m_BufferPool.getObject(handle).vkBuffer.vmaAllocation);
    m_BufferPool.destroy(handle);
}
} // namespace gore