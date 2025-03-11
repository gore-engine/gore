#pragma once

#include "Graphics/Vulkan/VulkanIncludes.h"

#include "Texture.h"
#include "BindLayout.h"

namespace gore::gfx
{
struct RawTextureBinding final
{
    uint32_t binding        = 0;
    vk::ImageView imageView = {};
    BindType bindType       = BindType::SampledImage;
    vk::Sampler sampler     = {};
};

struct RawBufferBinding final
{
    uint32_t binding  = 0;
    vk::Buffer buffer = {};
    uint32_t offset   = 0;
    uint32_t range    = 0;
    BindType bindType = BindType::UniformBuffer;
};

struct RawSamplerBinding final
{
    uint32_t binding    = 0;
    vk::Sampler sampler = {};
    BindType bindType   = BindType::Sampler;
};

struct RawBindGroupUpdateDesc final
{
    std::vector<RawTextureBinding> textures;
    std::vector<RawBufferBinding> buffers;
    std::vector<RawSamplerBinding> samplers;
};

} // namespace gore::gfx