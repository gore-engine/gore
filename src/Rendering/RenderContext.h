#pragma once

#include "Prefix.h"

#include "Graphics/Device.h"

#include "Graphics/Vulkan/VulkanIncludes.h"
#include "GraphicsCaching/ResourceCache.h"

#include "GraphicsResource.h"

#include "Rendering/Utils/GeometryUtils.h"
#include "Rendering/Components/MeshRenderer.h"

#include "CommandRing.h"
#include "Texture.h"
#include "Buffer.h"
#include "Sampler.h"
#include "BindGroup.h"
#include "DynamicBuffer.h"
#include "PipelineLayout.h"

#include "GraphicsPipelineDesc.h"
#include "Pipeline.h"
#include "RenderPass.h"
#include "RenderPassDesc.h"
#include "DescriptorPoolHolder.h"
#include "Pool.h"

#include "TransientBindGroupUpdateDesc.h"

#include <vector>

namespace gore::gfx
{
using namespace gore::renderer;

static constexpr uint32_t c_MaxRenderTargets = 8;
static constexpr uint32_t c_MaxViewports = 16;
static constexpr uint32_t c_MaxVertexAttributes = 16;
static constexpr uint32_t c_MaxBindingLayouts = 5;
static constexpr uint32_t c_MaxBindingsPerLayout = 128;
static constexpr uint32_t c_MaxVolatileConstantBuffersPerLayout = 6;
static constexpr uint32_t c_MaxVolatileConstantBuffers = 32;
static constexpr uint32_t c_MaxPushConstantSize = 128;
static constexpr uint32_t c_ConstantBufferOffsetSizeAlignment = 256; // Partially bound constant buffers must have offsets aligned to this and sizes multiple of this

enum PSOCreateRuntimeFlagBits
{
    PSO_CREATE_FLAG_NONE                     = 0,
    PSO_CREATE_FLAG_PREFER_RENDER_PASS       = 1 << 0,
    PSO_CREATE_FLAG_PREFER_SINGLE_SUBPASS    = 1 << 1,
    PSO_CREATE_FLAG_PREFER_DYNAMIC_RENDERING = 1 << 2,
    PSO_CREATE_FLAG_PREFER_NO_DEPENDENCIES   = 1 << 3,
    PSO_CREATE_FLAG_PREFER_PIPELINE_CACHE    = 1 << 4,
    PSO_CREATE_FLAG_PREFER_PIPELINE_LIBRARY  = 1 << 5,
    PSO_CREATE_FLAG_PREFER_RPS = PSO_CREATE_FLAG_PREFER_RENDER_PASS | PSO_CREATE_FLAG_PREFER_SINGLE_SUBPASS | PSO_CREATE_FLAG_PREFER_NO_DEPENDENCIES
};
 
struct RenderContextCreateInfo final
{
    const Device* device = nullptr;
    uint32_t flags       = PSO_CREATE_FLAG_NONE;
};

ENGINE_CLASS(RenderContext) final
{
    SINGLETON(RenderContext)
    
    public:
    RenderContext(const RenderContextCreateInfo& createInfo);
    ~RenderContext();
    
    void LoadMeshToMeshRenderer(const std::string& name, MeshRenderer& meshRenderer, uint32_t meshIndex = 0, ShaderChannel channel = ShaderChannel::Default);
    void LoadMesh();
    
    // Debug Utils
    void BeginDebugLabel(CommandBuffer& cmd, const char* label, float r = 1.0f, float g = 0.0f, float b = 0.0f);
    void InsertDebugLabel(CommandBuffer& cmd, const char* label, float r = 1.0f, float g = 0.0f, float b = 0.0f);
    void EndDebugLabel(CommandBuffer& cmd);
    
    // RenderPass
    RenderPass CreateRenderPass(RenderPassDesc&& desc);
    void DestroyRenderPass(RenderPass& renderPass);
    
    void DestroyRenderPass(RenderPass* renderPass);
    void BeginRenderPass(RenderPass* renderPass);
    void EndRenderPass();
    
    void PrepareRendering();
    
    std::unique_ptr<CommandRing> CreateCommandRing(const CommandRingCreateDesc& desc);
    void DestroyCommandRing(std::unique_ptr<CommandRing>& commandRing);
    void ResetCommandPool(CommandPool* commandPool);
    
    // Draw Call
    void DrawMesh(int instanceCount = 1, int firstInstance = 0);
    void DrawMeshIndirect();
    void DrawProcedural();
    void DrawProceduralIndirect();
    
    TextureHandle CreateTextureHandle(const std::string& name);
    TextureHandle CreateTextureHandle(TextureDesc&& desc);
    
    void DestroyTexture(TextureHandle handle);
    const Texture& GetTexture(TextureHandle handle);
    const TextureDesc& GetTextureDesc(TextureHandle handle);
    
    template <typename T>
    void CopyDataToBuffer(BufferHandle handle, const T& data)
    {
        CopyDataToBuffer(handle, &data, sizeof(T));
    }
    
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

    void ResetDescriptorPool(UpdateFrequency poolType = UpdateFrequency::PerFrame);

    BindGroupHandle CreateBindGroup(BindGroupDesc&& desc);
    void DestroyBindGroup(BindGroupHandle handle);
    const BindGroup& GetBindGroup(BindGroupHandle handle);
    const BindGroupDesc& GetBindGroupDesc(BindGroupHandle handle);

    TransientBindGroup CreateTransientBindGroup(BindGroupDesc&& desc);

    // BindGroup Update for RPSL
    void UpdateBindGroup(TransientBindGroup& bindGroup, BindGroupUpdateDesc&& desc, TransientBindGroupUpdateDesc&& transientDesc);
    void UpdateBindGroup(BindGroupHandle handle, BindGroupUpdateDesc&& desc, TransientBindGroupUpdateDesc&& transientDesc);

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

    Semaphore* CreateSemaphore();
    void DestroySemaphore(Semaphore& semaphore);

    Fence* CreateFence(bool signaled = true);
    void DestroyFence(Fence& fence);

    void Clear();

private:
    CommandPool* CreateCommandPool(const CommandPoolCreateDesc& desc);
    CommandBuffer* CreateCommandBuffer(const CommandBufferCreateDesc& desc);

    template <typename VkHPPObject>
    void SetObjectDebugName(const VkHPPObject& object, const std::string& name)
    {
        m_DevicePtr->SetName(*reinterpret_cast<uint64_t const *>(&object), VkHPPObject::objectType, name);
    }

    void DestroyTextureObject(const Texture& texture, const TextureDesc& desc);

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
    uint32_t m_PSOFlags;

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

    DescriptorPoolHolder m_DescriptorPool[(uint32_t)UpdateFrequency::Count];

    ResourceCache m_ResourceCache;

    vk::raii::CommandPool m_CommandPool;

    const Device* m_DevicePtr;
};

} // namespace gore::gfx