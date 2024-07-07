#include "RenderContext.h"
#include "Graphics/Device.h"
#include "RenderContextHelper.h"

#include "FileSystem/FileSystem.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Utilities/GLTFLoader.h"

#define VULKAN_DEVICE      (*m_DevicePtr->Get())
#define USE_STAGING_BUFFER 1

namespace gore::gfx
{
RenderContext::RenderContext(const Device* device) :
    m_DevicePtr(device),
    m_ShaderModulePool(),
    m_GraphicsPipelinePool(),
    m_BufferPool(),
    m_TexturePool(),
    m_CommandPool(VK_NULL_HANDLE)
{
    uint32_t queueFamilyIndex = device->GetQueueFamilyIndexByFlags(vk::QueueFlagBits::eGraphics);

    m_CommandPool = device->Get().createCommandPool({{}, queueFamilyIndex});
    device->SetName(m_CommandPool, "RenderContext CommandPool");
}

RenderContext::~RenderContext()
{
    clear();
}

void RenderContext::LoadMeshToMeshRenderer(const std::string& name, MeshRenderer& meshRenderer, uint32_t meshIndex, ShaderChannel channel)
{
    static const std::filesystem::path kGLTFFolder = FileSystem::GetResourceFolder() / "GLTF";
    auto gltfPath                                  = kGLTFFolder / name;
    GLTFLoader gltfLoader(*this);

    bool ret = gltfLoader.LoadMesh(meshRenderer, gltfPath.generic_string(), meshIndex, channel);

    if (ret == false)
    {
        LOG_STREAM(ERROR) << "Failed to load mesh at " << gltfPath << std::endl;
    }
}

BindLayout RenderContext::GetOrCreateBindLayout(const BindLayoutCreateInfo& createInfo)
{
    std::size_t hash{0u};
    utils::hash_combine(hash, createInfo);

    auto it = m_ResourceCache.bindLayouts.find(hash);
    if (it != m_ResourceCache.bindLayouts.end())
    {
        return it->second;
    }

    BindLayout bindLayout;
    std::vector<vk::DescriptorSetLayoutBinding> bindings;
    for (const auto& binding : createInfo.bindings)
    {
        vk::DescriptorSetLayoutBinding layoutBinding(
            binding.binding,
            VulkanHelper::GetVkDescriptorType(binding.type),
            binding.descriptorCount,
            VulkanHelper::GetVkShaderStageFlags(binding.stage),
            nullptr);

        bindings.push_back(layoutBinding);
    }

    bindLayout.layout = VULKAN_DEVICE.createDescriptorSetLayout({{}, static_cast<uint32_t>(bindings.size()), bindings.data()});

    // m_DevicePtr->SetName(bindLayout.layout, createInfo.name != nullptr ? createInfo.name : "NoNameBindLayout");

    m_ResourceCache.bindLayouts[hash] = bindLayout;

    return bindLayout;
}

void RenderContext::clear()
{
    m_ShaderModulePool.clear();

    auto& buffers = m_BufferPool.objects;
    for (auto& buffer : buffers)
    {
        ClearVulkanBuffer(m_DevicePtr->GetVmaAllocator(), buffer.vkBuffer, buffer.vmaAllocation);
    }
    m_BufferPool.clear();

    auto& textures     = m_TexturePool.objects;
    auto& textureDescs = m_TexturePool.objectDesc;
    for (int i = 0; i < textures.size(); i++)
    {
        DestroyTextureObject(textures[i], textureDescs[i].objDesc);
    }
    m_TexturePool.clear();

    auto& samplers = m_SamplerPool.objects;
    for (auto& sampler : samplers)
    {
        VULKAN_DEVICE.destroySampler(sampler.vkSampler);
    }
    m_SamplerPool.clear();

    ClearDescriptorPools();

    auto& pipelines = m_GraphicsPipelinePool.objects;
    for (auto& pipeline : pipelines)
    {
        VULKAN_DEVICE.destroyPipeline(pipeline.pipeline);
    }
    m_GraphicsPipelinePool.clear();

    m_CommandPool.clear();

    ClearCache(m_ResourceCache, VULKAN_DEVICE);
}

Buffer RenderContext::CreateStagingBuffer(const Device& device, void const* data, size_t size)
{
    VkBufferCreateInfo bufferInfo = {
        .sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size        = size,
        .usage       = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    VmaAllocationCreateInfo allocCreateInfo = {
        .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO,
    };

    Buffer buffer;
    buffer.vmaAllocator = device.GetVmaAllocator();

    VK_CHECK_RESULT(vmaCreateBuffer(device.GetVmaAllocator(), &bufferInfo, &allocCreateInfo, &buffer.vkBuffer, &buffer.vmaAllocation, &buffer.vmaAllocationInfo));

    void* mappedData = buffer.vmaAllocationInfo.pMappedData;
    memcpy(mappedData, data, size);

    return buffer;
}

vk::raii::CommandBuffer RenderContext::CreateCommandBuffer(vk::CommandBufferLevel level, bool begin)
{
    vk::CommandBufferAllocateInfo allocateInfo(*m_CommandPool, level, 1);
    vk::raii::CommandBuffer commandBuffer = std::move(m_DevicePtr->Get().allocateCommandBuffers(allocateInfo)[0]);

    if (begin)
    {
        commandBuffer.begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
    }

    return std::move(commandBuffer);
}

void RenderContext::FlushCommandBuffer(vk::raii::CommandBuffer& commandBuffer, vk::raii::Queue& queue)
{
    commandBuffer.end();

    vk::SubmitInfo submitInfo     = {};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers    = &*commandBuffer;

    vk::raii::Fence fence = m_DevicePtr->Get().createFence({});
    queue.submit(submitInfo, *fence);
    auto res = m_DevicePtr->Get().waitForFences(*fence, VK_TRUE, UINT64_MAX);
    VK_CHECK_RESULT(res);
}

void RenderContext::CreateDescriptorPools()
{
    vk::DescriptorPoolSize poolSizes[] = {
        {       vk::DescriptorType::eUniformBuffer, 1000},
        {vk::DescriptorType::eCombinedImageSampler, 1000},
        {       vk::DescriptorType::eStorageBuffer, 1000},
        {        vk::DescriptorType::eStorageImage, 1000},
        {             vk::DescriptorType::eSampler, 1000},
        {vk::DescriptorType::eUniformBufferDynamic, 100},
    };

    vk::DescriptorPoolCreateInfo poolCreateInfo(
        {},
        1,
        static_cast<uint32_t>(std::size(poolSizes)),
        poolSizes);

    m_DescriptorPool[(uint32_t)UpdateFrequency::None] = VULKAN_DEVICE.createDescriptorPool(poolCreateInfo);

    poolCreateInfo.maxSets                                = 100;
    m_DescriptorPool[(uint32_t)UpdateFrequency::PerFrame] = VULKAN_DEVICE.createDescriptorPool(poolCreateInfo);

    poolCreateInfo.maxSets                                = 100;
    m_DescriptorPool[(uint32_t)UpdateFrequency::PerBatch] = VULKAN_DEVICE.createDescriptorPool(poolCreateInfo);

    poolCreateInfo.maxSets                               = 100;
    m_DescriptorPool[(uint32_t)UpdateFrequency::PerDraw] = VULKAN_DEVICE.createDescriptorPool(poolCreateInfo);
}

void RenderContext::ClearDescriptorPools()
{
    VULKAN_DEVICE.destroyDescriptorPool(m_DescriptorPool[(uint32_t)UpdateFrequency::None]);
    VULKAN_DEVICE.destroyDescriptorPool(m_DescriptorPool[(uint32_t)UpdateFrequency::PerFrame]);
    VULKAN_DEVICE.destroyDescriptorPool(m_DescriptorPool[(uint32_t)UpdateFrequency::PerBatch]);
    VULKAN_DEVICE.destroyDescriptorPool(m_DescriptorPool[(uint32_t)UpdateFrequency::PerDraw]);
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

    vk::PipelineLayout pipelineLayout = GetOrCreatePipelineLayout(desc.bindLayouts).layout;

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

    createInfo.layout     = pipelineLayout;
    createInfo.renderPass = desc.renderPass;
    createInfo.subpass    = desc.subpassIndex;

    std::vector<VkFormat> colorFormats = VulkanHelper::GetVkFormats(desc.colorFormats);
    VkFormat depthFormat               = static_cast<VkFormat>(VulkanHelper::GetVkFormat(desc.depthFormat));
    VkFormat stencilFormat             = static_cast<VkFormat>(VulkanHelper::GetVkFormat(desc.stencilFormat));

    VkPipelineRenderingCreateInfoKHR rfInfo = {};

    if (desc.UseDynamicRendering())
    {
        rfInfo.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
        rfInfo.pNext                   = nullptr;
        rfInfo.colorAttachmentCount    = colorFormats.size();
        rfInfo.pColorAttachmentFormats = colorFormats.data();
        rfInfo.depthAttachmentFormat   = depthFormat;
        rfInfo.stencilAttachmentFormat = stencilFormat;

        createInfo.pNext = &rfInfo;
    }

    GraphicsPipeline graphicsPipeline(std::move(VULKAN_DEVICE.createGraphicsPipeline(nullptr, createInfo).value));
    graphicsPipeline.renderPass = desc.renderPass;
    graphicsPipeline.layout     = pipelineLayout;

    // m_DevicePtr->SetName(graphicsPipeline.pipeline, desc.debugName);

    return m_GraphicsPipelinePool.create(
        std::move(desc),
        std::move(graphicsPipeline));
}

const GraphicsPipeline& RenderContext::GetGraphicsPipeline(GraphicsPipelineHandle handle)
{
    return m_GraphicsPipelinePool.getObject(handle);
}

TextureHandle RenderContext::CreateTextureHandle(const std::string& name)
{
    static const std::filesystem::path kTextureFolder = FileSystem::GetResourceFolder() / "Textures";
    auto texturePath                                  = kTextureFolder / name;

    // TODO: change to use std::vector?

    int width, height, channel;
    stbi_uc* pixels = stbi_load(texturePath.generic_string().c_str(), &width, &height, &channel, STBI_rgb_alpha);

    if (pixels == nullptr)
    {
        LOG_STREAM(ERROR) << "RenderSystem CreateTextureHandle: Failed to load texture: " << name << std::endl;
        return TextureHandle();
    }

    TextureHandle handle = CreateTexture({.debugName = name.c_str(),
                                          .width     = static_cast<uint32_t>(width),
                                          .height    = static_cast<uint32_t>(height),
                                          .data      = pixels,
                                          .dataSize  = static_cast<uint32_t>(width * height * 4)});

    stbi_image_free(pixels);

    return handle;
}

TextureHandle RenderContext::CreateTexture(TextureDesc&& desc)
{
    VkImageCreateInfo imageInfo = {
        .sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType     = VulkanHelper::GetVkImageType(desc.type),
        .format        = static_cast<VkFormat>(VulkanHelper::GetVkFormat(desc.format)),
        .extent        = {desc.width, desc.height, 1},
        .mipLevels     = static_cast<uint32_t>(desc.numMips),
        .arrayLayers   = static_cast<uint32_t>(desc.numLayers),
        .samples       = VK_SAMPLE_COUNT_1_BIT,
        .tiling        = VK_IMAGE_TILING_OPTIMAL,
        .usage         = VulkanHelper::GetVkImageUsageFlags(desc.usage),
        .sharingMode   = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED,
    };

    VmaAllocationCreateInfo allocCreateInfo = {
        .flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
    };

    Texture texture;
    VK_CHECK_RESULT(vmaCreateImage(m_DevicePtr->GetVmaAllocator(), &imageInfo, &allocCreateInfo, &texture.image, &texture.vmaAllocation, &texture.vmaAllocationInfo));

    vk::ImageViewCreateInfo imageViewInfo;
    imageViewInfo.image                           = texture.image;
    imageViewInfo.viewType                        = VulkanHelper::GetVkImageViewType(desc.type);
    imageViewInfo.format                          = VulkanHelper::GetVkFormat(desc.format);
    imageViewInfo.subresourceRange.aspectMask     = vk::ImageAspectFlagBits::eColor;
    imageViewInfo.subresourceRange.baseMipLevel   = 0;
    imageViewInfo.subresourceRange.levelCount     = desc.numMips;
    imageViewInfo.subresourceRange.baseArrayLayer = 0;
    imageViewInfo.subresourceRange.layerCount     = desc.numLayers;

    if (HasFlag(desc.usage, TextureUsageBits::Sampled))
    {
        texture.srv = VULKAN_DEVICE.createImageView(imageViewInfo);
    }

    TextureHandle handle = m_TexturePool.create(std::move(desc), std::move(texture));

    if (desc.data == nullptr && desc.dataSize == 0)
        return handle;

    assert(desc.data != nullptr && desc.dataSize > 0);

    vk::raii::Queue queue = m_DevicePtr->Get().getQueue(m_DevicePtr->GetQueueFamilyIndexByFlags(vk::QueueFlagBits::eGraphics), 0);

    vk::raii::CommandBuffer cmd = CreateCommandBuffer(vk::CommandBufferLevel::ePrimary, true);

    CopyDataToTexture(handle, desc.data, desc.dataSize);

    FlushCommandBuffer(cmd, queue);

    return handle;
}

void RenderContext::DestroyTextureObject(const Texture& texture, const TextureDesc& desc)
{
    DestroyVulkanTexture(m_DevicePtr->GetVmaAllocator(), texture.image, texture.vmaAllocation);

    if (HasFlag(desc.usage, TextureUsageBits::Sampled))
        VULKAN_DEVICE.destroyImageView(texture.srv);

    if (HasFlag(desc.usage, TextureUsageBits::Storage))
    {
        for (int i = 0; i < desc.numMips; i++)
            VULKAN_DEVICE.destroyImageView(texture.uav[i]);
    }

    if (HasFlag(desc.usage, TextureUsageBits::DepthStencil))
        VULKAN_DEVICE.destroyImageView(texture.srvStencil);
}

void RenderContext::DestroyTexture(TextureHandle handle)
{
    auto& textureDesc = m_TexturePool.getObjectDesc(handle);
    auto& texture     = m_TexturePool.getObject(handle);

    DestroyTextureObject(texture, textureDesc);

    m_TexturePool.destroy(handle);
}

const Texture& RenderContext::GetTexture(TextureHandle handle)
{
    return m_TexturePool.getObject(handle);
}

const TextureDesc& RenderContext::GetTextureDesc(TextureHandle handle)
{
    return m_TexturePool.getObjectDesc(handle);
}

void RenderContext::CopyDataToBuffer(BufferHandle handle, const void* data, size_t size)
{
    auto buffer = m_BufferPool.getObject(handle);

#if USE_STAGING_BUFFER
    Buffer stagingBuffer = CreateStagingBuffer(*m_DevicePtr, data, size);

    vk::raii::Queue queue = m_DevicePtr->Get().getQueue(m_DevicePtr->GetQueueFamilyIndexByFlags(vk::QueueFlagBits::eGraphics), 0);

    vk::raii::CommandBuffer cmd = CreateCommandBuffer(vk::CommandBufferLevel::ePrimary, true);

    cmd.copyBuffer(stagingBuffer.vkBuffer, buffer.vkBuffer, vk::BufferCopy(0, 0, size));

    FlushCommandBuffer(cmd, queue);

    ClearVulkanBuffer(m_DevicePtr->GetVmaAllocator(), stagingBuffer.vkBuffer, stagingBuffer.vmaAllocation);
#endif
}

void RenderContext::CopyDataToTexture(TextureHandle handle, const void* data, size_t size)
{
    auto texture     = m_TexturePool.getObject(handle);
    auto textureDesc = m_TexturePool.getObjectDesc(handle);

    Buffer stagingBuffer = CreateStagingBuffer(*m_DevicePtr, data, size);

    vk::raii::Queue queue = m_DevicePtr->Get().getQueue(m_DevicePtr->GetQueueFamilyIndexByFlags(vk::QueueFlagBits::eGraphics), 0);

    vk::raii::CommandBuffer cmd = CreateCommandBuffer(vk::CommandBufferLevel::ePrimary, true);

    vk::ImageSubresourceRange subresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);

    vk::Image image = texture.image;

    VulkanHelper::ImageLayoutTransition(cmd, image, vk::ImageLayout::ePreinitialized, vk::ImageLayout::eTransferDstOptimal, subresourceRange);

    cmd.copyBufferToImage(stagingBuffer.vkBuffer, image, vk::ImageLayout::eTransferDstOptimal, vk::BufferImageCopy(0, 0, 0, vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1), vk::Offset3D(0, 0, 0), vk::Extent3D(textureDesc.width, textureDesc.height, 1)));

    VulkanHelper::ImageLayoutTransition(cmd, image, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, subresourceRange);

    FlushCommandBuffer(cmd, queue);

    ClearVulkanBuffer(m_DevicePtr->GetVmaAllocator(), stagingBuffer.vkBuffer, stagingBuffer.vmaAllocation);
}

BufferHandle RenderContext::CreateBuffer(BufferDesc&& desc)
{
    uint32_t byteSize = desc.byteSize;

    VkBufferCreateInfo bufferInfo = VulkanHelper::GetVkBufferCreateInfo(desc);

    VmaAllocationCreateInfo allocCreateInfo = VulkanHelper::GetVmaAllocationCreateInfo(desc);

    Buffer buffer;
    buffer.vmaAllocator = m_DevicePtr->GetVmaAllocator();

    VK_CHECK_RESULT(vmaCreateBuffer(m_DevicePtr->GetVmaAllocator(), &bufferInfo, &allocCreateInfo, &buffer.vkBuffer, &buffer.vmaAllocation, &buffer.vmaAllocationInfo));

    VkBufferDeviceAddressInfo bufferDeviceAddressInfo = {
        .sType  = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .buffer = buffer.vkBuffer,
    };

    buffer.vkDeviceAddress = m_DevicePtr->Get().getBufferAddress(bufferDeviceAddressInfo);

    m_DevicePtr->SetName(reinterpret_cast<uint64_t>(buffer.vkBuffer), vk::ObjectType::eBuffer, desc.debugName);

    BufferHandle handle = m_BufferPool.create(std::move(desc), std::move(buffer));

    if (desc.data == nullptr)
        return handle;

    if (desc.memUsage == MemoryUsage::CPU)
    {
        SetBufferData(GetBuffer(handle), static_cast<const uint8_t*>(desc.data), byteSize, 0);
    }
    else
    {
        CopyDataToBuffer(handle, desc.data, byteSize);
    }

    return handle;
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
    auto buffer = m_BufferPool.getObject(handle);

    ClearVulkanBuffer(m_DevicePtr->GetVmaAllocator(), buffer.vkBuffer, buffer.vmaAllocation);
    m_BufferPool.destroy(handle);
}

SamplerHandle RenderContext::CreateSampler(SamplerDesc&& desc)
{
    vk::SamplerCreateInfo samplerCreateInfo{};

    samplerCreateInfo.minFilter = VulkanHelper::GetVkFilter(desc.minFilter);
    samplerCreateInfo.magFilter = VulkanHelper::GetVkFilter(desc.magFilter);

    samplerCreateInfo.mipmapMode   = VulkanHelper::GetVkMipmapMode(desc.mipmapMode);
    samplerCreateInfo.addressModeU = VulkanHelper::GetVkAddressMode(desc.addressModeU);
    samplerCreateInfo.addressModeV = VulkanHelper::GetVkAddressMode(desc.addressModeV);
    samplerCreateInfo.addressModeW = VulkanHelper::GetVkAddressMode(desc.addressModeW);

    samplerCreateInfo.mipLodBias       = desc.mipLodBias;
    samplerCreateInfo.compareOp        = VulkanHelper::GetVkCompareOp(desc.compareOp);
    samplerCreateInfo.minLod           = desc.minLod;
    samplerCreateInfo.maxLod           = desc.maxLod;
    samplerCreateInfo.maxAnisotropy    = desc.maxAnisotropy;
    samplerCreateInfo.anisotropyEnable = desc.anisotropyEnable;
    samplerCreateInfo.borderColor      = vk::BorderColor::eFloatOpaqueWhite;

    Sampler sampler = {.vkSampler = VULKAN_DEVICE.createSampler(samplerCreateInfo)};

    return m_SamplerPool.create(std::move(desc), std::move(sampler));
}

const SamplerDesc& RenderContext::GetSamplerDesc(SamplerHandle handle)
{
    return m_SamplerPool.getObjectDesc(handle);
}

const Sampler& RenderContext::GetSampler(SamplerHandle handle)
{
    return m_SamplerPool.getObject(handle);
}

void RenderContext::DestroySampler(SamplerHandle handle)
{
    auto sampler = m_SamplerPool.getObject(handle).vkSampler;

    VULKAN_DEVICE.destroySampler(sampler);
    m_SamplerPool.destroy(handle);
}

BindGroupHandle RenderContext::createBindGroup(BindGroupDesc&& desc)
{
    vk::DescriptorSetLayout setLayout = desc.bindLayout->layout;
    vk::DescriptorPool pool           = m_DescriptorPool[(uint32_t)desc.updateFrequency];

    vk::DescriptorSet descriptorSet = VULKAN_DEVICE.allocateDescriptorSets({pool, 1, &setLayout})[0];

    std::vector<vk::WriteDescriptorSet> writeDescriptorSets;
    writeDescriptorSets.reserve(desc.buffers.size() + desc.textures.size() + desc.samplers.size());

    std::vector<vk::DescriptorBufferInfo> bufferInfos;
    bufferInfos.reserve(desc.buffers.size());
    for (const auto& buffer : desc.buffers)
    {
        const BufferDesc& bufferDesc = GetBufferDesc(buffer.handle);
        const Buffer& bufferInfo     = GetBuffer(buffer.handle);

        bufferInfos.push_back({GetBuffer(buffer.handle).vkBuffer, buffer.byteOffset, bufferDesc.byteSize});

        vk::WriteDescriptorSet writeDescriptorSet(
            descriptorSet,
            buffer.binding,
            0,
            1,
            VulkanHelper::GetVkDescriptorType(buffer.bindType),
            nullptr,
            &bufferInfos.back(),
            nullptr);

        writeDescriptorSets.push_back(writeDescriptorSet);
    }

    std::vector<vk::DescriptorImageInfo> imageInfos;
    imageInfos.reserve(desc.textures.size());
    for (const auto& textureBinding : desc.textures)
    {
        TextureHandle handle = textureBinding.handle;

        const TextureDesc& textureDesc = GetTextureDesc(handle);
        const Texture& textureInfo     = GetTexture(handle);

        vk::ImageView imageView = VK_NULL_HANDLE;

        if (HasFlag(textureBinding.usage, TextureUsageBits::Sampled))
        {
            imageView = textureInfo.srv;
        }

        if (HasFlag(textureBinding.usage, TextureUsageBits::Storage))
        {
            imageView = textureInfo.uav[0];
        }

        vk::DescriptorImageInfo imageInfo;
        imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        imageInfo.imageView   = imageView;
        imageInfo.sampler     = textureBinding.bindType == BindType::CombinedSampledImage ? GetSampler(textureBinding.samplerHandle).vkSampler : VK_NULL_HANDLE;

        imageInfos.push_back(imageInfo);

        vk::WriteDescriptorSet writeDescriptorSet(
            descriptorSet,
            textureBinding.binding,
            textureBinding.arrayIndex,
            1,
            VulkanHelper::GetVkDescriptorType(textureBinding.bindType),
            &imageInfos.back(),
            nullptr,
            nullptr);

        writeDescriptorSets.push_back(writeDescriptorSet);
    }

    std::vector<vk::DescriptorImageInfo> samplerInfos;
    samplerInfos.reserve(desc.samplers.size());
    for (const auto& samplerBinding : desc.samplers)
    {
        SamplerHandle handle = samplerBinding.handle;

        const SamplerDesc& samplerDesc = GetSamplerDesc(handle);
        const Sampler& samplerInfo     = GetSampler(handle);

        vk::DescriptorImageInfo imageInfo;
        imageInfo.sampler = samplerInfo.vkSampler;

        samplerInfos.push_back(imageInfo);

        vk::WriteDescriptorSet writeDescriptorSet(
            descriptorSet,
            samplerBinding.binding,
            0,
            1,
            VulkanHelper::GetVkDescriptorType(samplerBinding.bindType),
            &samplerInfos.back(),
            nullptr,
            nullptr);

        writeDescriptorSets.push_back(writeDescriptorSet);
    }

    VULKAN_DEVICE.updateDescriptorSets(writeDescriptorSets, {});

    return m_BindGroupPool.create(
        std::move(desc),
        BindGroup{descriptorSet});
}

void RenderContext::DestroyBindGroup(BindGroupHandle handle)
{
    auto bindGroupDesc = m_BindGroupPool.getObjectDesc(handle);
    auto bindGroup = m_BindGroupPool.getObject(handle);

    vk::DescriptorPool pool = m_DescriptorPool[(uint32_t)bindGroupDesc.updateFrequency];
    VULKAN_DEVICE.freeDescriptorSets(pool, bindGroup.set);

    m_BindGroupPool.destroy(handle);
}

const BindGroup& RenderContext::GetBindGroup(BindGroupHandle handle)
{
    return m_BindGroupPool.getObject(handle);
}

const BindGroupDesc& RenderContext::GetBindGroupDesc(BindGroupHandle handle)
{
    return m_BindGroupPool.getObjectDesc(handle);
}

void RenderContext::PrepareRendering()
{
    CreateDescriptorPools();
}

PipelineLayout RenderContext::GetOrCreatePipelineLayout(const std::vector<BindLayout>& createInfo)
{
    std::size_t hash{0u};
    utils::hash_combine(hash, createInfo);

    auto it = m_ResourceCache.pipelineLayouts.find(hash);
    if (it != m_ResourceCache.pipelineLayouts.end())
    {
        return it->second;
    }

    std::vector<vk::DescriptorSetLayout> layouts;
    layouts.reserve(createInfo.size());
    for (const auto& layout : createInfo)
    {
        layouts.push_back(layout.layout);
    }

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo(
        {},
        static_cast<uint32_t>(layouts.size()),
        layouts.data(),
        0,
        nullptr);

    PipelineLayout pipelineLayout;
    pipelineLayout.layout = VULKAN_DEVICE.createPipelineLayout(pipelineLayoutInfo);

    m_ResourceCache.pipelineLayouts[hash] = pipelineLayout;

    return pipelineLayout;
}
DynamicBufferHandle RenderContext::CreateDynamicBuffer(DynamicBufferDesc&& desc)
{
    std::vector<Binding> bindings = {
        {0, BindType::DynamicUniformBuffer, 1, ShaderStage::Vertex},
    };

    BindLayoutCreateInfo bindLayoutCreateInfo = {
        .name     = desc.debugName,
        .bindings = bindings,
    };

    BindLayout bindLayout = GetOrCreateBindLayout(bindLayoutCreateInfo);

    vk::DescriptorSetLayout setLayout = bindLayout.layout;

    vk::DescriptorPool pool = m_DescriptorPool[(uint32_t)UpdateFrequency::PerDraw];

    vk::DescriptorSetAllocateInfo allocInfo(
        pool,
        1,
        &setLayout);

    vk::DescriptorSet descriptorSet = VULKAN_DEVICE.allocateDescriptorSets(allocInfo)[0];

    std::vector<vk::WriteDescriptorSet> writeDescriptorSets;
    writeDescriptorSets.reserve(1);

    const BufferDesc& bufferDesc = GetBufferDesc(desc.buffer);
    const Buffer& bufferInfo     = GetBuffer(desc.buffer);

    vk::DescriptorBufferInfo bufferInfoDesc = {
        bufferInfo.vkBuffer,
        0,
        bufferDesc.byteSize};

    vk::WriteDescriptorSet writeDescriptorSet(
        descriptorSet,
        0,
        0,
        1,
        vk::DescriptorType::eUniformBufferDynamic,
        nullptr,
        &bufferInfoDesc,
        nullptr);

    writeDescriptorSets.push_back(writeDescriptorSet);

    VULKAN_DEVICE.updateDescriptorSets(writeDescriptorSets, {});

    return m_DynamicBufferPool.create(
        std::move(desc),
        DynamicBuffer{descriptorSet, setLayout, desc.offset});
}

const DynamicBufferDesc& RenderContext::GetDynamicBufferDesc(DynamicBufferHandle handle)
{
    return m_DynamicBufferPool.getObjectDesc(handle);
}

const DynamicBuffer& RenderContext::GetDynamicBuffer(DynamicBufferHandle handle)
{
    return m_DynamicBufferPool.getObject(handle);
}

void RenderContext::DestroyDynamicBuffer(DynamicBufferHandle handle)
{
    auto dynamicBuffer = m_DynamicBufferPool.getObject(handle);

    vk::DescriptorPool pool = m_DescriptorPool[(uint32_t)UpdateFrequency::PerDraw];
    VULKAN_DEVICE.freeDescriptorSets(pool, dynamicBuffer.set);

    m_DynamicBufferPool.destroy(handle);
}
} // namespace gore::gfx