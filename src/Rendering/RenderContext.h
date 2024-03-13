#pragma once

#include "Prefix.h"

#include "DummyVertex.h"

#include "Graphics/Vulkan/VulkanIncludes.h"
#include "Graphics/VulkanBuffer.h"

#include "GraphicsResourceDesc.h"
#include "GraphicsResource.h"

#include "BindGroup.h"

#include "GraphicsPipelineDesc.h"
#include "Pipeline.h"
#include "RenderPass.h"
#include "Pool.h"

namespace gore::gfx
{

class Device;

class RenderContext final
{
    // TODO: actually we can copy this class??
    NON_COPYABLE(RenderContext);

public:
    RenderContext(const Device* device);
    ~RenderContext();

    // RenderPass
    RenderPass* CreateRenderPass(const RenderPassDesc& desc);
    RenderPass& GetCurrentRenderPass();
    void DestroyRenderPass(RenderPass* renderPass);
    void BeginRenderPass(RenderPass* renderPass);
    void EndRenderPass();

    // Draw Call
    void DrawMesh(int instanceCount = 1, int firstInstance = 0);
    void DrawMeshIndirect();
    void DrawProcedural();
    void DrawProceduralIndirect();

    TextureHandle createTexture(TextureDesc&& desc);

    template <typename T>
    void CopyDataToTexture(TextureHandle handle, const std::vector<T>& data)
    {
        CopyDataToTexture(handle, data.data(), data.size() * sizeof(T));
    }
    void CopyDataToTexture(TextureHandle handle, const void* data, size_t size);

    BufferHandle CreateBuffer(BufferDesc&& desc);
    const BufferDesc& GetBufferDesc(BufferHandle handle);
    const Buffer& GetBuffer(BufferHandle handle);
    void DestroyBuffer(BufferHandle handle);

    SamplerHandle createSampler(const SamplerDesc& desc);
    BindGroupHandle createBindGroup(const BindGroupDesc& desc);

    ShaderModuleHandle createShaderModule(ShaderModuleDesc&& desc);
    const ShaderModuleDesc& getShaderModuleDesc(ShaderModuleHandle handle);
    const ShaderModule& getShaderModule(ShaderModuleHandle handle);
    void destroyShaderModule(ShaderModuleHandle handle);

    GraphicsPipelineHandle CreateGraphicsPipeline(GraphicsPipelineDesc&& desc);
    const GraphicsPipeline& GetGraphicsPipeline(GraphicsPipelineHandle handle);

    void destroyTexture(TextureHandle handle);
    void destroySampler(SamplerHandle handle);
    void destroyBindGroup(BindGroupHandle handle);
    void destroyPipeline(GraphicsPipelineHandle handle);

    void clear();
private:
    template <typename T>
    static VulkanBuffer CreateStagintBuffer(const Device& device, std::vector<T> const& data)
    {
        return CreateStagingBuffer(device, data.data(), data.size() * sizeof(T));
    }

    static VulkanBuffer CreateStagingBuffer(const Device& device, void const* data, size_t size);

    vk::raii::CommandBuffer CreateCommandBuffer(vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary, bool begin = true);
    void FlushCommandBuffer(vk::raii::CommandBuffer& commandBuffer, vk::raii::Queue& queue);

private:
    using ShaderModulePool     = Pool<ShaderModuleDesc, ShaderModule>;
    using BufferPool           = Pool<BufferDesc, Buffer>;
    using TexturePool          = Pool<TextureDesc, Texture>;
    using GraphicsPipelinePool = Pool<GraphicsPipelineDesc, GraphicsPipeline>;

    ShaderModulePool m_ShaderModulePool;
    BufferPool m_BufferPool;
    TexturePool m_TexturePool;
    GraphicsPipelinePool m_GraphicsPipelinePool;

    vk::raii::CommandPool m_CommandPool;

    const Device* m_DevicePtr;
};

} // namespace gore::gfx