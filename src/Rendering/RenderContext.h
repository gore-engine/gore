#pragma once

#include "Prefix.h"

#include "DummyVertex.h"

#include "Graphics/Vulkan/VulkanIncludes.h"

#include "GraphicsResourceDesc.h"
#include "GraphicsResource.h"

#include "BindGroup.h"

#include "GraphicsPipelineDesc.h"
#include "Pipeline.h"
#include "RenderPass.h"
#include "Pool.h"

namespace gore
{

namespace gfx
{
class Device;
}

class RenderContext final
{
    // TODO: actually we can copy this class??
    NON_COPYABLE(RenderContext);

public:
    RenderContext(const gfx::Device* device);
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

    TextureHandle createTexture(const TextureDesc& desc);

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
    using ShaderModulePool = Pool<ShaderModuleDesc, ShaderModule>;
    using BufferPool       = Pool<BufferDesc, Buffer>;
    using GraphicsPipelinePool = Pool<GraphicsPipelineDesc, GraphicsPipeline>;

    ShaderModulePool m_ShaderModulePool;
    BufferPool m_BufferPool;
    GraphicsPipelinePool m_GraphicsPipelinePool;

    const gfx::Device* m_DevicePtr;
};

} // namespace gore