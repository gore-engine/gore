#pragma once

#include "Prefix.h"

#include "GraphicsResourceDesc.h"
#include "GrpahicsResource.h"

#include "BindGroup.h"

#include "GraphicsPipelineDesc.h"
#include "GraphicsPipeline.h"

#include "Pool.h"

namespace gore
{
class RenderContext final
{
    // TODO: actually we can copy this class??
    NON_COPYABLE(RenderContext);

public:
    RenderContext() = default;

    TextureHandle createTexture(const TextureDesc& desc);
    BufferHandle createBuffer(const BufferDesc& desc);
    ShaderModuleHandle createShaderModule(const ShaderModuleDesc& desc);
    SamplerHandle createSampler(const SamplerDesc& desc);
    BindGroupHandle createBindGroup(const BindGroupDesc& desc);
    PipelineHandle createPipeline(const PipelineDesc& desc);

    void destroyTexture(TextureHandle handle);
    void destroyBuffer(BufferHandle handle);
    void destroyShaderModule(ShaderModuleHandle handle);
    void destroySampler(SamplerHandle handle);
    void destroyBindGroup(BindGroupHandle handle);
    void destroyPipeline(PipelineHandle handle);

    void clear();

private:
    Pool<TextureHandle, Texture*> m_texturePool                = {};
    Pool<BufferHandle, Buffer*> m_bufferPool                   = {};
    Pool<ShaderModuleHandle, ShaderModule*> m_shaderModulePool = {};
    Pool<SamplerHandle, Sampler*> m_samplerPool                = {};
    Pool<BindGroupHandle, BindGroup*> m_bindGroupPool          = {};
    Pool<PipelineHandle, Pipeline*> m_pipelinePool             = {};
};

} // namespace gore