#include "RenderContext.h"
#include "Graphics/Device.h"

#include "RenderContextHelper.h"

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

    VkBufferCreateInfo bufferInfo = vulkanHelper::GetVkBufferCreateInfo(desc);

    VmaAllocationCreateInfo allocCreateInfo = vulkanHelper::GetVmaAllocationCreateInfo(desc);

    VulkanBuffer buffer;
    buffer.vmaAllocator = m_DevicePtr->GetVmaAllocator();

    VK_CHECK_RESULT(vmaCreateBuffer(m_DevicePtr->GetVmaAllocator(), &bufferInfo, &allocCreateInfo, &buffer.vkBuffer, &buffer.vmaAllocation, &buffer.vmaAllocationInfo));

    VkBufferDeviceAddressInfo bufferDeviceAddressInfo = {
        .sType  = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .buffer = buffer.vkBuffer,
    };

    buffer.vkDeviceAddress = m_DevicePtr->Get().getBufferAddress(bufferDeviceAddressInfo);

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
    using namespace gfx;
    
    auto buffer = m_BufferPool.getObject(handle).vkBuffer;

    vmaDestroyBuffer(m_DevicePtr->GetVmaAllocator(), buffer.vkBuffer, buffer.vmaAllocation);
    m_BufferPool.destroy(handle);
}
} // namespace gore