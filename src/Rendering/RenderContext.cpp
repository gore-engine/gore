#include "RenderContext.h"
#include "RenderContextHelper.h"

#include "FileSystem/FileSystem.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Utilities/GLTFLoader.h"

#define VULKAN_DEVICE      (*m_DevicePtr->Get())
#define USE_STAGING_BUFFER 1

namespace gore::gfx
{
SINGLETON_IMPL(RenderContext)

RenderContext::RenderContext(const RenderContextCreateInfo& createInfo) :
    m_DevicePtr(createInfo.device),
    m_ShaderModulePool(),
    m_GraphicsPipelinePool(),
    m_BufferPool(),
    m_TexturePool(),
    m_CommandPool(VK_NULL_HANDLE),
    m_PSOFlags(createInfo.flags)
{
    uint32_t queueFamilyIndex = m_DevicePtr->GetQueueFamilyIndexByFlags(vk::QueueFlagBits::eGraphics);

    m_CommandPool = m_DevicePtr->Get().createCommandPool({{}, queueFamilyIndex});
    m_DevicePtr->SetName(m_CommandPool, "RenderContext CommandPool");

    g_Instance = this;
}

RenderContext::~RenderContext()
{
    Clear();

    g_Instance = nullptr;
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

    SetObjectDebugName(bindLayout.layout, createInfo.name != nullptr ? createInfo.name : "NoName_BindLayout");

    m_ResourceCache.bindLayouts[hash] = bindLayout;

    return bindLayout;
}

void RenderContext::Clear()
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

    Buffer buffer = {};
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
        {        vk::DescriptorType::eSampledImage, 1000},
        {       vk::DescriptorType::eStorageBuffer, 1000},
        {        vk::DescriptorType::eStorageImage, 1000},
        {             vk::DescriptorType::eSampler, 1000},
        {vk::DescriptorType::eUniformBufferDynamic,  100},
    };

    vk::DescriptorPoolCreateInfo poolCreateInfo(
        {},
        1024,
        static_cast<uint32_t>(std::size(poolSizes)),
        poolSizes);

    for (int i = 0; i < FramedDescriptorPool::c_MaxFrames; i++)
    {
        m_FramedDescriptorPool.pools.emplace_back(static_cast<DescriptorPoolHolder>(VULKAN_DEVICE.createDescriptorPool(poolCreateInfo)));
        SetObjectDebugName(m_FramedDescriptorPool.pools[i].pool, "RenderContext PerFrame DescriptorPool " + std::to_string(i));
    }

    m_DescriptorPool[(uint32_t)UpdateFrequency::Persistent] = static_cast<DescriptorPoolHolder>(VULKAN_DEVICE.createDescriptorPool(poolCreateInfo));
    SetObjectDebugName(m_DescriptorPool[(uint32_t)UpdateFrequency::Persistent].pool, "RenderContext Persistent DescriptorPool");
    
    m_DescriptorPool[(uint32_t)UpdateFrequency::PerBatch] = static_cast<DescriptorPoolHolder>(VULKAN_DEVICE.createDescriptorPool(poolCreateInfo));
    SetObjectDebugName(m_DescriptorPool[(uint32_t)UpdateFrequency::PerBatch].pool, "RenderContext PerBatch DescriptorPool");

    m_DescriptorPool[(uint32_t)UpdateFrequency::PerDraw] = static_cast<DescriptorPoolHolder>(VULKAN_DEVICE.createDescriptorPool(poolCreateInfo));
    SetObjectDebugName(m_DescriptorPool[(uint32_t)UpdateFrequency::PerDraw].pool, "RenderContext PerDraw DescriptorPool");

    m_EmptySetLayout = VULKAN_DEVICE.createDescriptorSetLayout({});
}

void RenderContext::ClearDescriptorPools()
{
    VULKAN_DEVICE.destroyDescriptorSetLayout(m_EmptySetLayout);

    for (int i = 0; i < FramedDescriptorPool::c_MaxFrames; i++)
    {
        VULKAN_DEVICE.destroyDescriptorPool(m_FramedDescriptorPool.pools[i]);
    }

    VULKAN_DEVICE.destroyDescriptorPool(m_DescriptorPool[(uint32_t)UpdateFrequency::Persistent]);
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

    const DynamicBuffer* dynamicBuffer = nullptr;
    if (desc.dynamicBuffer.empty() == false)
        dynamicBuffer = &GetDynamicBuffer(desc.dynamicBuffer);

    vk::PipelineLayout pipelineLayout = GetOrCreatePipelineLayout(desc.bindLayouts, dynamicBuffer).layout;

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

    bool useDynamicRendering = desc.UseDynamicRendering() && (m_PSOFlags & PSO_CREATE_FLAG_PREFER_DYNAMIC_RENDERING) != 0;
    bool useRenderPass       = m_PSOFlags & PSO_CREATE_FLAG_PREFER_RENDER_PASS;
    bool useSingleSubpass    = m_PSOFlags & PSO_CREATE_FLAG_PREFER_SINGLE_SUBPASS;
    bool useNoDependencies   = m_PSOFlags & PSO_CREATE_FLAG_PREFER_NO_DEPENDENCIES;

    VkPipelineRenderingCreateInfoKHR rfInfo = {};

    if (useDynamicRendering)
    {
        rfInfo.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
        rfInfo.pNext                   = nullptr;
        rfInfo.colorAttachmentCount    = colorFormats.size();
        rfInfo.pColorAttachmentFormats = colorFormats.data();
        rfInfo.depthAttachmentFormat   = depthFormat;
        rfInfo.stencilAttachmentFormat = stencilFormat;

        createInfo.pNext = &rfInfo;
    }

    GraphicsPipeline graphicsPipeline;
    graphicsPipeline.pipeline = VULKAN_DEVICE.createGraphicsPipeline(nullptr, createInfo).value;
    graphicsPipeline.layout   = pipelineLayout;

    SetObjectDebugName(graphicsPipeline.pipeline, desc.debugName);

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

    TextureHandle handle = CreateTextureHandle({.debugName = name.c_str(),
                                          .width     = static_cast<uint32_t>(width),
                                          .height    = static_cast<uint32_t>(height),
                                          .data      = pixels,
                                          .dataSize  = static_cast<uint32_t>(width * height * 4)});

    stbi_image_free(pixels);

    return handle;
}

TextureHandle RenderContext::CreateTextureHandle(TextureDesc&& desc)
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

    SetObjectDebugName(sampler.vkSampler, desc.debugName);

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

void RenderContext::ResetDescriptorPool(UpdateFrequency poolType)
{
    if (poolType == UpdateFrequency::PerFrame)
    {
        m_FramedDescriptorPool.currentPoolIndex = (m_FramedDescriptorPool.currentPoolIndex + 1) % FramedDescriptorPool::c_MaxFrames;
        VULKAN_DEVICE.resetDescriptorPool(m_FramedDescriptorPool.pools[m_FramedDescriptorPool.currentPoolIndex], {});
        return;
    }
    
    assert(poolType < UpdateFrequency::Count && poolType != UpdateFrequency::PerFrame);
    VULKAN_DEVICE.resetDescriptorPool(m_DescriptorPool[(uint32_t)poolType], {});
}

BindGroupHandle RenderContext::CreateBindGroup(BindGroupDesc&& desc)
{
    vk::DescriptorSetLayout setLayout = desc.bindLayout->layout;
    vk::DescriptorPool pool           = GetDescriptorPool(desc.updateFrequency);

    vk::DescriptorSet descriptorSet = VULKAN_DEVICE.allocateDescriptorSets({pool, 1, &setLayout})[0];

    SetObjectDebugName(descriptorSet, desc.debugName);

    std::vector<vk::WriteDescriptorSet> writeDescriptorSets;
    writeDescriptorSets.reserve(desc.buffers.size() + desc.textures.size() + desc.samplers.size());

    std::vector<vk::DescriptorBufferInfo> bufferInfos;
    bufferInfos.reserve(desc.buffers.size());
    for (const auto& buffer : desc.buffers)
    {
        const BufferDesc& bufferDesc = GetBufferDesc(buffer.handle);
        const Buffer& bufferInfo     = GetBuffer(buffer.handle);

        bufferInfos.push_back({GetBuffer(buffer.handle).vkBuffer,
                               buffer.offset,
                               buffer.range == 0 ? bufferDesc.byteSize : buffer.range});

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
    auto bindGroup     = m_BindGroupPool.getObject(handle);

    vk::DescriptorPool pool = GetDescriptorPool(bindGroupDesc.updateFrequency);
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

TransientBindGroup RenderContext::CreateTransientBindGroup(BindGroupDesc&& desc)
{
    vk::DescriptorSetLayout setLayout = desc.bindLayout->layout;
    vk::DescriptorPool pool           = GetDescriptorPool(desc.updateFrequency);

    vk::DescriptorSet descriptorSet = VULKAN_DEVICE.allocateDescriptorSets({pool, 1, &setLayout})[0];

    SetObjectDebugName(descriptorSet, desc.debugName);

    return TransientBindGroup{descriptorSet};    
}

void RenderContext::PrepareRendering()
{
    CreateDescriptorPools();
}

PipelineLayout RenderContext::GetOrCreatePipelineLayout(const std::vector<BindLayout>& createInfo, const DynamicBuffer* dynamicBuffer)
{
    std::size_t hash{0u};
    utils::hash_combine(hash, createInfo);

    auto it = m_ResourceCache.pipelineLayouts.find(hash);
    if (it != m_ResourceCache.pipelineLayouts.end())
    {
        return it->second;
    }

    uint32_t layoutCount = static_cast<uint32_t>(dynamicBuffer != nullptr ? 4 : createInfo.size());

    std::vector<vk::DescriptorSetLayout> layouts;
    layouts.reserve(layoutCount);

    for (const auto& layout : createInfo)
    {
        layouts.push_back(layout.layout);
    }

    int fakeSetLayout = layoutCount - createInfo.size() - 1;
    for (int i = 0; i < fakeSetLayout; i++)
    {
        layouts.push_back(m_EmptySetLayout);
    }

    if (dynamicBuffer != nullptr)
    {
        layouts.push_back(dynamicBuffer->layout);
    }

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo(
        {},
        layoutCount,
        layouts.data(),
        0,
        nullptr);

    PipelineLayout pipelineLayout;
    pipelineLayout.layout = VULKAN_DEVICE.createPipelineLayout(pipelineLayoutInfo);

    m_ResourceCache.pipelineLayouts[hash] = pipelineLayout;

    return pipelineLayout;
}

void RenderContext::UpdateBindGroup(BindGroupHandle handle
    , BindGroupUpdateDesc&& bindGroupDesc
    , TransientBindGroupUpdateDesc&& transientDesc)
{
    auto descriptorSet = GetBindGroup(handle).set;

    TransientBindGroup bindGroup = TransientBindGroup{descriptorSet};

    UpdateBindGroup(bindGroup, std::move(bindGroupDesc), std::move(transientDesc));
}

void RenderContext::UpdateBindGroup(TransientBindGroup& bindGroup, BindGroupUpdateDesc&& bindGroupDesc, TransientBindGroupUpdateDesc&& transientDesc)
{
    auto descriptorSet = bindGroup.descriptorSet;

    std::vector<vk::DescriptorImageInfo> descriptorImageInfos;
    descriptorImageInfos.reserve(c_MaxBindingsPerLayout);

    std::vector<vk::DescriptorBufferInfo> descriptorBufferInfos;
    descriptorBufferInfos.reserve(c_MaxBindingsPerLayout);
    
    std::vector<vk::WriteDescriptorSet> writeDescriptorSets;
    writeDescriptorSets.reserve(c_MaxBindingsPerLayout);

    auto generateWriteDescriptorData = 
        [&](uint32_t binding
        , vk::DescriptorType descriptorType
        , vk::DescriptorImageInfo* imageInfo
        , vk::DescriptorBufferInfo* bufferInfo
        , vk::BufferView* texelBufferView
        , const void* pNext = nullptr)
        {
            writeDescriptorSets.push_back(
                vk::WriteDescriptorSet()
                .setDstSet(descriptorSet)
                .setDstBinding(binding)
                .setDstArrayElement(0)
                .setDescriptorCount(1)
                .setDescriptorType(descriptorType)
                .setPImageInfo(imageInfo)
                .setPBufferInfo(bufferInfo)
                .setPTexelBufferView(texelBufferView)
                .setPNext(pNext));                
        };
    
    // update persistent bindgroup update
    for (const auto& textureBinding : bindGroupDesc.textures)
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

        descriptorImageInfos.push_back(imageInfo);

        generateWriteDescriptorData(
            textureBinding.binding,
            VulkanHelper::GetVkDescriptorType(textureBinding.bindType),
            &descriptorImageInfos.back(),
            nullptr,
            nullptr);
    }
    for (const auto& bufferBinding : bindGroupDesc.buffers)
    {
        const BufferDesc& bufferDesc = GetBufferDesc(bufferBinding.handle);
        const Buffer& bufferInfo     = GetBuffer(bufferBinding.handle);

        descriptorBufferInfos.push_back({GetBuffer(bufferBinding.handle).vkBuffer,
                                         bufferBinding.offset,
                                         bufferBinding.range == 0 ? bufferDesc.byteSize : bufferBinding.range});

        generateWriteDescriptorData(
            bufferBinding.binding,
            VulkanHelper::GetVkDescriptorType(bufferBinding.bindType),
            nullptr,
            &descriptorBufferInfos.back(),
            nullptr);
    }
    for (const auto& samplerBinding : bindGroupDesc.samplers)
    {
        SamplerHandle handle = samplerBinding.handle;

        const SamplerDesc& samplerDesc = GetSamplerDesc(handle);
        const Sampler& samplerInfo     = GetSampler(handle);

        vk::DescriptorImageInfo imageInfo;
        imageInfo.sampler = samplerInfo.vkSampler;

        descriptorImageInfos.push_back(imageInfo);

        generateWriteDescriptorData(
            samplerBinding.binding,
            VulkanHelper::GetVkDescriptorType(samplerBinding.bindType),
            &descriptorImageInfos.back(),
            nullptr,
            nullptr);
    }
    // update transient bindgroup update
    for (const auto& textureBinding : transientDesc.textures)
    {
        vk::DescriptorImageInfo imageInfo;
        imageInfo.imageLayout = textureBinding.imageLayout;
        imageInfo.imageView   = textureBinding.imageView;

        descriptorImageInfos.push_back(imageInfo);

        generateWriteDescriptorData(
            textureBinding.binding,
            textureBinding.descriptorType,
            &descriptorImageInfos.back(),
            nullptr,
            nullptr);
    }

    for (const auto& bufferBinding : transientDesc.buffers)
    {
        descriptorBufferInfos.push_back({bufferBinding.buffer,
                                         bufferBinding.offset,
                                         bufferBinding.range});

        generateWriteDescriptorData(
            bufferBinding.binding,
            bufferBinding.descriptorType,
            nullptr,
            &descriptorBufferInfos.back(),
            nullptr);
    }

    for (const auto& samplerBinding : transientDesc.samplers)
    {
        vk::DescriptorImageInfo imageInfo;
        imageInfo.sampler = samplerBinding.sampler;

        descriptorImageInfos.push_back(imageInfo);

        generateWriteDescriptorData(
            samplerBinding.binding,
            vk::DescriptorType::eSampler,
            &descriptorImageInfos.back(),
            nullptr,
            nullptr);
    }

    VULKAN_DEVICE.updateDescriptorSets(writeDescriptorSets, {});
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

    vk::DescriptorPool pool = GetDescriptorPool(UpdateFrequency::Persistent);

    vk::DescriptorSetAllocateInfo allocInfo(
        pool,
        1,
        &setLayout);

    vk::DescriptorSet descriptorSet = VULKAN_DEVICE.allocateDescriptorSets(allocInfo)[0];

    SetObjectDebugName(descriptorSet, desc.debugName);

    std::vector<vk::WriteDescriptorSet> writeDescriptorSets;
    writeDescriptorSets.reserve(1);

    const BufferDesc& bufferDesc = GetBufferDesc(desc.buffer);
    const Buffer& bufferInfo     = GetBuffer(desc.buffer);

    vk::DescriptorBufferInfo bufferInfoDesc = {
        bufferInfo.vkBuffer,
        desc.offset,
        desc.range == 0 ? bufferDesc.byteSize : desc.range};

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

    vk::DescriptorPool pool = GetDescriptorPool(UpdateFrequency::Persistent);
    VULKAN_DEVICE.freeDescriptorSets(pool, dynamicBuffer.set);

    m_DynamicBufferPool.destroy(handle);
}

CommandPool* RenderContext::CreateCommandPool(const CommandPoolCreateDesc& desc)
{
    CommandPool* commandPool = new CommandPool();

    vk::CommandPoolCreateInfo poolInfo(
        {},
        desc.queueFamilyIndex);

    if (desc.transient)
    {
        poolInfo.flags |= vk::CommandPoolCreateFlagBits::eTransient;
    }

    commandPool->cmdPool = VULKAN_DEVICE.createCommandPool(poolInfo);

    return commandPool;
}

CommandBuffer* RenderContext::CreateCommandBuffer(const CommandBufferCreateDesc& desc)
{
    CommandBuffer* commandBuffer = new CommandBuffer();

    vk::CommandBufferAllocateInfo allocInfo(
        desc.cmdPool->cmdPool,
        desc.secondary ? vk::CommandBufferLevel::eSecondary : vk::CommandBufferLevel::ePrimary,
        1);

    commandBuffer->cmdBuffer = VULKAN_DEVICE.allocateCommandBuffers(allocInfo).front();

#if ENGINE_DEBUG
    SetObjectDebugName(commandBuffer->cmdBuffer, desc.debugName);
#endif

    return commandBuffer;
}

std::unique_ptr<CommandRing> RenderContext::CreateCommandRing(const CommandRingCreateDesc& desc)
{
    std::unique_ptr<CommandRing> commandRing = std::make_unique<CommandRing>();
    commandRing->poolCount                   = desc.cmdPoolCount;
    commandRing->cmdBufferCountPerPool       = desc.cmdBufferCountPerPool;

    commandRing->hasSyncObjects = desc.addSyncObjects;

    for (int poolIndex = 0; poolIndex < desc.cmdPoolCount; poolIndex++)
    {
        commandRing->cmdPools[poolIndex] = CreateCommandPool({desc.queueFamilyIndex, false});

        for (int cmdBufferIndex = 0; cmdBufferIndex < desc.cmdBufferCountPerPool; cmdBufferIndex++)
        {
            CommandBufferCreateDesc cmdBufferDesc;
            cmdBufferDesc.cmdPool   = commandRing->cmdPools[poolIndex];
            cmdBufferDesc.secondary = desc.secondary;
#if ENGINE_DEBUG
            std::string debugName = desc.debugName;
            debugName += "_" + std::to_string(poolIndex) + "_" + std::to_string(cmdBufferIndex);
            cmdBufferDesc.debugName = debugName.c_str();
#endif
            commandRing->cmdBuffers[poolIndex][cmdBufferIndex] = CreateCommandBuffer(cmdBufferDesc);

            if (desc.addSyncObjects)
            {
                commandRing->semaphores[poolIndex][cmdBufferIndex] = CreateSemaphore();
                commandRing->fences[poolIndex][cmdBufferIndex]     = CreateFence();
            }
        }
    }

    return commandRing;
}

Semaphore* RenderContext::CreateSemaphore()
{
    Semaphore* semaphore = new Semaphore();
    semaphore->semaphore = VULKAN_DEVICE.createSemaphore({});
    return semaphore;
}

Fence* RenderContext::CreateFence(bool signaled)
{
    Fence* fence = new Fence();
    fence->fence = VULKAN_DEVICE.createFence({signaled ? vk::FenceCreateFlagBits::eSignaled : vk::FenceCreateFlags()});
    return fence;
}

void RenderContext::DestroyCommandRing(std::unique_ptr<CommandRing>& commandRing)
{
    for (int poolIndex = 0; poolIndex < commandRing->poolCount; poolIndex++)
    {
        VULKAN_DEVICE.destroyCommandPool(commandRing->cmdPools[poolIndex]->cmdPool);

        if (commandRing->hasSyncObjects == false)
            continue;

        for (int cmdBufferIndex = 0; cmdBufferIndex < commandRing->cmdBufferCountPerPool; cmdBufferIndex++)
        {
            VULKAN_DEVICE.destroySemaphore(commandRing->semaphores[poolIndex][cmdBufferIndex]->semaphore);
            VULKAN_DEVICE.destroyFence(commandRing->fences[poolIndex][cmdBufferIndex]->fence);
        }
    }
}

void RenderContext::ResetCommandPool(CommandPool* commandPool)
{
    VULKAN_DEVICE.resetCommandPool(commandPool->cmdPool);
}

void RenderContext::BeginDebugLabel(CommandBuffer& cmd, const char* label, float r, float g, float b)
{
#if ENGINE_DEBUG
    vk::DebugUtilsLabelEXT labelInfo;
    labelInfo.pLabelName = label;
    labelInfo.color[0]   = r;
    labelInfo.color[1]   = g;
    labelInfo.color[2]   = b;
    labelInfo.color[3]   = 1.0f;

    cmd.cmdBuffer.beginDebugUtilsLabelEXT(labelInfo);
#endif
}

void RenderContext::EndDebugLabel(CommandBuffer& cmd)
{
#if ENGINE_DEBUG
    cmd.cmdBuffer.endDebugUtilsLabelEXT();
#endif
}

void RenderContext::InsertDebugLabel(CommandBuffer& cmd, const char* label, float r, float g, float b)
{
#if ENGINE_DEBUG
    cmd.cmdBuffer.insertDebugUtilsLabelEXT({
        label,
        {r, g, b, 1.0f}
    });
#endif
}

void RenderContext::DestroySemaphore(Semaphore& semaphore)
{
    VULKAN_DEVICE.destroySemaphore(semaphore.semaphore);
    semaphore.semaphore = nullptr;
}

void RenderContext::DestroyFence(Fence& fence)
{
    VULKAN_DEVICE.destroyFence(fence.fence);
    fence.fence = nullptr;
}

RenderPass RenderContext::CreateRenderPass(RenderPassDesc&& desc)
{
    bool hasDepthStencil = desc.depthFormat != GraphicsFormat::Undefined || desc.stencilFormat != GraphicsFormat::Undefined;
    bool hasColor        = desc.colorFormats.size() > 0;

    std::vector<VkFormat> colorFormats = VulkanHelper::GetVkFormats(desc.colorFormats);
    vk::Format depthFormat             = VulkanHelper::GetVkFormat(desc.depthFormat);

    std::vector<vk::AttachmentDescription> attachments;
    attachments.reserve(colorFormats.size() + (hasDepthStencil ? 1 : 0));

    for (int i = 0; i < colorFormats.size(); i++)
    {
        vk::AttachmentDescription attachment(
            {},
            (vk::Format)colorFormats[i],
            vk::SampleCountFlagBits::e1,
            vk::AttachmentLoadOp::eClear,
            vk::AttachmentStoreOp::eStore,
            vk::AttachmentLoadOp::eDontCare,
            vk::AttachmentStoreOp::eDontCare,
            vk::ImageLayout::eUndefined,
            vk::ImageLayout::ePresentSrcKHR);

        attachments.push_back(attachment);
    }

    if (hasDepthStencil)
    {
        vk::AttachmentDescription attachment(
            {},
            depthFormat,
            vk::SampleCountFlagBits::e1,
            vk::AttachmentLoadOp::eDontCare,
            vk::AttachmentStoreOp::eStore,
            vk::AttachmentLoadOp::eClear,
            vk::AttachmentStoreOp::eStore,
            vk::ImageLayout::eDepthStencilAttachmentOptimal,
            vk::ImageLayout::eDepthStencilAttachmentOptimal);

        attachments.push_back(attachment);
    }

    vk::AttachmentReference colorAttachmentRefs[8]; // max color attachments
    for (int i = 0; i < colorFormats.size(); i++)
    {
        colorAttachmentRefs[i] = {static_cast<uint32_t>(i), vk::ImageLayout::eColorAttachmentOptimal};
    }

    vk::AttachmentReference depthAttachmentRef;
    if (hasDepthStencil)
    {
        depthAttachmentRef = {static_cast<uint32_t>(colorFormats.size()), vk::ImageLayout::eDepthStencilAttachmentOptimal};
    }

    vk::SubpassDescription subpass(
        {},
        vk::PipelineBindPoint::eGraphics,
        0,
        nullptr,
        static_cast<uint32_t>(colorFormats.size()),
        hasColor? colorAttachmentRefs : nullptr,
        nullptr,
        hasDepthStencil ? &depthAttachmentRef : nullptr,
        0,
        nullptr);

    vk::RenderPassCreateInfo renderPassInfo(
        {},
        static_cast<uint32_t>(attachments.size()),
        attachments.data(),
        1,
        &subpass);

    return {VULKAN_DEVICE.createRenderPass(renderPassInfo)};
}

void RenderContext::DestroyRenderPass(RenderPass& renderPass)
{
    VULKAN_DEVICE.destroyRenderPass(renderPass.renderPass);
}
} // namespace gore::gfx