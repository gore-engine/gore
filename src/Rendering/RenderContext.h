#pragma once

#include "Prefix.h"

#include "Graphics/Vulkan/VulkanIncludes.h"

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
    RenderContext(vk::raii::Device* device);
    ~RenderContext();

    TextureHandle createTexture(const TextureDesc& desc);
    BufferHandle createBuffer(const BufferDesc& desc);
    SamplerHandle createSampler(const SamplerDesc& desc);
    BindGroupHandle createBindGroup(const BindGroupDesc& desc);
    PipelineHandle createPipeline(const PipelineDesc& desc);

    void destroyTexture(TextureHandle handle);
    void destroyBuffer(BufferHandle handle);
    void destroySampler(SamplerHandle handle);
    void destroyBindGroup(BindGroupHandle handle);
    void destroyPipeline(PipelineHandle handle);

    void clear();

private:
    Pool<TextureDesc, Texture*> m_texturePool                = {};
    Pool<BufferDesc, Buffer*> m_bufferPool                   = {};
    Pool<SamplerDesc, Sampler*> m_samplerPool                = {};
    Pool<BindGroupDesc, BindGroup*> m_bindGroupPool          = {};
    Pool<PipelineDesc, Pipeline*> m_pipelinePool             = {};

    vk::raii::Device* m_device = nullptr;
};

} // namespace gore