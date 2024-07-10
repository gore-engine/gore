#pragma once

#include "Prefix.h"

#include "Graphics/Vulkan/VulkanIncludes.h"
#include "GraphicsCaching/ResourceCache.h"

#include "GraphicsResource.h"

#include "Rendering/Utils/GeometryUtils.h"
#include "Rendering/Components/MeshRenderer.h"

#include "Texture.h"
#include "Buffer.h"
#include "Sampler.h"
#include "BindGroup.h"
#include "DynamicBuffer.h"
#include "PipelineLayout.h"

#include "GraphicsPipelineDesc.h"
#include "Pipeline.h"
#include "RenderPass.h"
#include "Pool.h"

#include <vector>

namespace gore::gfx
{

class Device;

ENGINE_CLASS(RenderContext) final
{
    // TODO: actually we can copy this class??
    NON_COPYABLE(RenderContext);

public:
    RenderContext(const Device* device);
    ~RenderContext();

    void LoadMeshToMeshRenderer(const std::string& name, MeshRenderer& meshRenderer, uint32_t meshIndex = 0, ShaderChannel channel = ShaderChannel::Default);
    void LoadMesh();

    // RenderPass
    RenderPass* CreateRenderPass(const RenderPassDesc& desc);
    RenderPass& GetCurrentRenderPass();
    void DestroyRenderPass(RenderPass* renderPass);
    void BeginRenderPass(RenderPass* renderPass);
    void EndRenderPass();

    void PrepareRendering();

    // Draw Call
    void DrawMesh(int instanceCount = 1, int firstInstance = 0);
    void DrawMeshIndirect();
    void DrawProcedural();
    void DrawProceduralIndirect();

    TextureHandle CreateTextureHandle(const std::string& name);

    void DestroyTexture(TextureHandle handle);
    const Texture& GetTexture(TextureHandle handle);
    const TextureDesc& GetTextureDesc(TextureHandle handle);

    template <typename T>
    void CopyDataToTexture(TextureHandle handle, const std::vector<T>& data)
    {
        CopyDataToTexture(handle, data.data(), data.size() * sizeof(T));
    }

    template <typename T>
    void CopyDataToBuffer(BufferHandle handle, const std::vector<T>& data)
    {
        CopyDataToBuffer(handle, data.data(), data.size() * sizeof(T));
    }

    BufferHandle CreateBuffer(BufferDesc&& desc);
    const BufferDesc& GetBufferDesc(BufferHandle handle);
    const Buffer& GetBuffer(BufferHandle handle);
    void DestroyBuffer(BufferHandle handle);

    SamplerHandle CreateSampler(SamplerDesc&& desc);
    const SamplerDesc& GetSamplerDesc(SamplerHandle handle);
    const Sampler& GetSampler(SamplerHandle handle);
    void DestroySampler(SamplerHandle handle);

    BindGroupHandle CreateBindGroup(BindGroupDesc&& desc);
    void DestroyBindGroup(BindGroupHandle handle);
    const BindGroup& GetBindGroup(BindGroupHandle handle);
    const BindGroupDesc& GetBindGroupDesc(BindGroupHandle handle);

    DynamicBufferHandle CreateDynamicBuffer(DynamicBufferDesc&& desc);
    const DynamicBufferDesc& GetDynamicBufferDesc(DynamicBufferHandle handle);
    const DynamicBuffer& GetDynamicBuffer(DynamicBufferHandle handle);
    void DestroyDynamicBuffer(DynamicBufferHandle handle);

    ShaderModuleHandle createShaderModule(ShaderModuleDesc&& desc);
    const ShaderModuleDesc& getShaderModuleDesc(ShaderModuleHandle handle);
    const ShaderModule& getShaderModule(ShaderModuleHandle handle);
    void destroyShaderModule(ShaderModuleHandle handle);

    GraphicsPipelineHandle CreateGraphicsPipeline(GraphicsPipelineDesc&& desc);
    const GraphicsPipeline& GetGraphicsPipeline(GraphicsPipelineHandle handle);

    BindLayout GetOrCreateBindLayout(const BindLayoutCreateInfo& createInfo);
    PipelineLayout GetOrCreatePipelineLayout(const std::vector<BindLayout>& createInfo, const DynamicBuffer* dynamicBuffer = nullptr);

    void Clear();

private:
    void DestroyTextureObject(const Texture& texture, const TextureDesc& desc);
    TextureHandle CreateTexture(TextureDesc&& desc);

    template <typename T>
    static Buffer CreateStagingBuffer(const Device& device, std::vector<T> const& data)
    {
        return CreateStagingBuffer(device, data.data(), data.size() * sizeof(T));
    }

    static Buffer CreateStagingBuffer(const Device& device, void const* data, size_t size);
    
    void CopyDataToBuffer(BufferHandle handle, const void* data, size_t size);
    void CopyDataToTexture(TextureHandle handle, const void* data, size_t size);

    vk::raii::CommandBuffer CreateCommandBuffer(vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary, bool begin = true);
    void FlushCommandBuffer(vk::raii::CommandBuffer& commandBuffer, vk::raii::Queue& queue);

    void CreateDescriptorPools();
    void ClearDescriptorPools();

private:
    using ShaderModulePool     = Pool<ShaderModuleDesc, ShaderModule>;
    using BufferPool           = Pool<BufferDesc, Buffer>;
    using TexturePool          = Pool<TextureDesc, Texture>;
    using GraphicsPipelinePool = Pool<GraphicsPipelineDesc, GraphicsPipeline>;
    using SamplerPool          = Pool<SamplerDesc, Sampler>;
    using BindGroupPool        = Pool<BindGroupDesc, BindGroup>;
    using DynamicBufferPool    = Pool<DynamicBufferDesc, DynamicBuffer>;

    ShaderModulePool m_ShaderModulePool;
    BufferPool m_BufferPool;
    TexturePool m_TexturePool;
    GraphicsPipelinePool m_GraphicsPipelinePool;
    SamplerPool m_SamplerPool;
    BindGroupPool m_BindGroupPool;
    DynamicBufferPool m_DynamicBufferPool;

    vk::DescriptorSetLayout m_EmptySetLayout;

    vk::DescriptorPool m_DescriptorPool[(uint32_t)UpdateFrequency::Count];

    ResourceCache m_ResourceCache;

    vk::raii::CommandPool m_CommandPool;

    const Device* m_DevicePtr;
};

} // namespace gore::gfx