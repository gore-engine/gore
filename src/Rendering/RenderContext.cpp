#include "RenderContext.h"
#include "Graphics/Device.h"

#include "RenderContextHelper.h"

namespace gore
{
RenderContext::RenderContext(const gfx::Device* device) :
    m_DevicePtr(device),
    m_ShaderModulePool(),
    m_CommandPool(VK_NULL_HANDLE),
{
    device->GetQueueFamilyProperties();

    vk::CommandPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .queueFamilyIndex = device->GetQueueFamilyIndex(),
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
    };

    m_CommandPool = device->Get().createCommandPool(poolInfo);
}

RenderContext::~RenderContext()
{
    clear();
}

void RenderContext::clear()
{
    m_ShaderModulePool.clear();
    m_GraphicsPipelinePool.clear();
}

gfx::VulkanBuffer RenderContext::CreateStagingBuffer(const gfx::Device& device, void const* data, size_t size)
{
    using namespace gfx;

    VkBufferCreateInfo bufferInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    VmaAllocationCreateInfo allocCreateInfo = {
        .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
        .usage = VMA_MEMORY_USAGE_CPU_ONLY,
    };
    
    VulkanBuffer buffer;
    buffer.vmaAllocator = device.GetVmaAllocator();

    VK_CHECK_RESULT(vmaCreateBuffer(device.GetVmaAllocator(), &bufferInfo, &allocCreateInfo, &buffer.vkBuffer, &buffer.vmaAllocation, &buffer.vmaAllocationInfo));

    return buffer;
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

GraphicsPipelineHandle RenderContext::CreateGraphicsPipeline(GraphicsPipelineDesc&& desc)
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


    auto [attributes, bindings] = VulkanHelper::GetVkVertexInputState(desc.vertexBufferBindings);
    vk::PipelineVertexInputStateCreateInfo vertexInputState({}, bindings, attributes, nullptr);

    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState = VulkanHelper::GetVkInputAssemblyState(desc);

    vk::PipelineViewportStateCreateInfo viewportState = VulkanHelper::GetVkViewportState(desc);

    vk::PipelineRasterizationStateCreateInfo rasterizeState = VulkanHelper::GetVkRasterizeState(desc);

    vk::PipelineMultisampleStateCreateInfo multisampleState = VulkanHelper::GetVkMultisampleState(desc);

    vk::PipelineDepthStencilStateCreateInfo depthStencilState = VulkanHelper::GetVkDepthStencilState(desc);

    std::vector<vk::PipelineColorBlendAttachmentState> vkColorBlendAttachments(desc.blendState.attachments.size());
    for (int i = 0; i < desc.blendState.attachments.size(); i++)
    {
        vkColorBlendAttachments[i] = VulkanHelper::GetVkColorBlendAttachmentState(desc.blendState.attachments[i]);
    }

    vk::PipelineColorBlendStateCreateInfo colorBlendState = {
        {},
        desc.blendState.enable,
        VulkanHelper::GetVkLogicOp(desc.blendState.logicOp),
        static_cast<uint32_t>(vkColorBlendAttachments.size()),
        vkColorBlendAttachments.data(),
        {1.0f, 1.0f, 1.0f, 1.0f}
    };

    std::vector<vk::DynamicState> dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor, vk::DynamicState::eLineWidth, vk::DynamicState::eDepthBias, vk::DynamicState::eBlendConstants, vk::DynamicState::eDepthBounds, vk::DynamicState::eStencilCompareMask, vk::DynamicState::eStencilWriteMask, vk::DynamicState::eStencilReference};

    vk::PipelineDynamicStateCreateInfo dynamicState = {
        {},
        dynamicStates};
        
    vk::GraphicsPipelineCreateInfo createInfo;
    createInfo.stageCount          = 2;
    createInfo.pStages             = shaderStages.data();
    createInfo.pVertexInputState   = &vertexInputState;
    createInfo.pInputAssemblyState = &inputAssemblyState;
    createInfo.pViewportState      = &viewportState;
    createInfo.pRasterizationState = &rasterizeState;
    createInfo.pMultisampleState   = &multisampleState;
    createInfo.pDepthStencilState  = &depthStencilState;
    createInfo.pColorBlendState    = &colorBlendState;
    createInfo.pDynamicState       = &dynamicState;

    createInfo.layout     = desc.pipelineLayout;
    createInfo.renderPass = desc.renderPass;
    createInfo.subpass    = desc.subpassIndex;

    std::vector<VkFormat> colorFormats = VulkanHelper::GetVkFormats(desc.colorFormats);
    VkFormat depthFormat = static_cast<VkFormat>(VulkanHelper::GetVkFormat(desc.depthFormat));
    VkFormat stencilFormat = static_cast<VkFormat>(VulkanHelper::GetVkFormat(desc.stencilFormat));

    VkPipelineRenderingCreateInfoKHR rfInfo = {};

    if (desc.UseDynamicRendering())
    {
        rfInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
        rfInfo.pNext = nullptr;
        rfInfo.colorAttachmentCount = colorFormats.size();
        rfInfo.pColorAttachmentFormats = colorFormats.data();
        rfInfo.depthAttachmentFormat = depthFormat;
        rfInfo.stencilAttachmentFormat = stencilFormat;

        createInfo.pNext = &rfInfo;
    }

    GraphicsPipeline graphicsPipeline(std::move(m_DevicePtr->Get().createGraphicsPipeline(nullptr, createInfo)));
    graphicsPipeline.renderPass = desc.renderPass;
    graphicsPipeline.layout     = desc.pipelineLayout;

    m_DevicePtr->SetName(graphicsPipeline.pipeline, desc.debugName);

    return m_GraphicsPipelinePool.create(
        std::move(desc),
        std::move(graphicsPipeline));
}

const GraphicsPipeline& RenderContext::GetGraphicsPipeline(GraphicsPipelineHandle handle)
{
    return m_GraphicsPipelinePool.getObject(handle);
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

    m_DevicePtr->SetName(reinterpret_cast<uint64_t>(buffer.vkBuffer), vk::ObjectType::eBuffer,  desc.debugName);

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