#pragma once

#include "Handle.h"
#include "GraphicsResource.h"
#include "Texture.h"
#include "Buffer.h"
#include "BindLayout.h"
#include "Sampler.h"

#include <vector>

namespace gore::gfx
{

enum class UpdateFrequency : uint8_t
{
    Persistent,
    PerBatch,
    PerDraw,
    Count,
    PerFrame // deprecated
};

struct TextureBinding final
{
    uint32_t binding       = 0;
    TextureHandle handle   = {};
    TextureUsageBits usage = TextureUsageBits::Sampled;
    uint8_t arrayIndex     = 0;
    uint8_t mipLevel       = 0;
    BindType bindType      = BindType::SampledImage;
    // if sampleType == Combined then this is the sampler binding
    SamplerHandle samplerHandle = {};
};

/// @brief Texture binding for bindless textures
struct ArrayTexturesBinding final
{
    uint32_t binding                   = 0;
    uint32_t count                     = 0;
    std::vector<TextureHandle> handles = {};
    TextureUsageBits usage             = TextureUsageBits::Sampled;
};

/// @brief Buffer handle with byte offset
struct BufferBinding final
{
    uint32_t binding    = 0;
    BufferHandle handle = {};
    uint32_t offset = 0;
    uint32_t range      = 0;
    BindType bindType   = BindType::UniformBuffer;
};

// FIXME: not a good padding
struct SamplerBinding final
{
    uint32_t binding     = 0;
    SamplerHandle handle = {};
    BindType bindType    = BindType::Sampler;
};

struct BindGroupUpdateDesc final
{
    std::vector<TextureBinding> textures = {};
    std::vector<BufferBinding> buffers   = {};
    std::vector<SamplerBinding> samplers  = {};
};

struct BindGroupDesc final
{
    const char* debugName                = nullptr;
    UpdateFrequency updateFrequency      = UpdateFrequency::Persistent;
    std::vector<TextureBinding> textures = {};
    std::vector<BufferBinding> buffers   = {};
    std::vector<SamplerBinding> samplers  = {};
    const BindLayout* bindLayout         = nullptr;
};

struct BindGroup final
{
    /// @brief Vulkan : DescriptorSet
    vk::DescriptorSet set;
};

using BindGroupHandle = Handle<BindGroup>;
} // namespace gore::gfx