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

GraphicsPipelineHandle RenderContext::createGraphicsPipeline(const GraphicsPipelineDesc& desc)
{
    using namespace std;

    vk::raii::ShaderModule vs = m_DevicePtr->Get().createShaderModule(vk::ShaderModuleCreateInfo(
        {},
        desc.VS.byteSize,
        reinterpret_cast<const uint32_t*>(desc.VS.byteCode)));

    vk::raii::ShaderModule ps = m_DevicePtr->Get().createShaderModule(vk::ShaderModuleCreateInfo(
        {},
        desc.PS.byteSize,
        reinterpret_cast<const uint32_t*>(desc.PS.byteCode)));

    vector<vk::PipelineShaderStageCreateInfo> shaderStages = {
        vk::PipelineShaderStageCreateInfo(
            {},
            vk::ShaderStageFlagBits::eVertex,
            *vs,
            desc.VS.entryFunc),
        vk::PipelineShaderStageCreateInfo(
            {},
            vk::ShaderStageFlagBits::eFragment,
            *ps,
            desc.PS.entryFunc)};

    vk::GraphicsPipelineCreateInfo createInfo;
    createInfo.stageCount = 2;
    createInfo.pStages    = shaderStages.data();

    auto [attributes, bindings] = vulkanHelper::GetVkVertexInputState(desc.vertexBufferBindings);
    vk::PipelineVertexInputStateCreateInfo vertexInputState({}, bindings, attributes, nullptr);
    createInfo.pVertexInputState = &vertexInputState;
    
    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState = vulkanHelper::GetVkInputAssemblyState(desc);
    createInfo.pInputAssemblyState = &inputAssemblyState;

    vk::PipelineViewportStateCreateInfo viewportState = vulkanHelper::GetVkViewportState(desc);
    createInfo.pViewportState = &viewportState;

    vk::PipelineRasterizationStateCreateInfo rasterizeState = vulkanHelper::GetVkRasterizeState(desc);
    createInfo.pRasterizationState = &rasterizeState;

    return GraphicsPipelineHandle();
}

BufferHandle RenderContext::CreateBuffer(BufferDesc&& desc)
{
    using namespace gfx;

    uint32_t byteSize = desc.byteSize;

    VkBufferCreateInfo bufferInfo = VulkanHelper::GetVkBufferCreateInfo(desc);

    VmaAllocationCreateInfo allocCreateInfo = VulkanHelper::GetVmaAllocationCreateInfo(desc);

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