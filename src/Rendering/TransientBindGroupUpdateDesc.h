#pragma once

#include "Graphics/Vulkan/VulkanIncludes.h"

#include "Texture.h"
#include "BindLayout.h"

namespace gore::gfx
{
struct TransientBindGroup
{
    vk::DescriptorSet descriptorSet = {};
};

struct TransientTextureBinding final
{
    uint32_t binding        = 0;
    vk::ImageView imageView = {};
    vk::ImageLayout imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    vk::DescriptorType descriptorType = vk::DescriptorType::eSampledImage;
    vk::Sampler sampler     = {};
};

struct TransientBufferBinding final
{
    uint32_t binding  = 0;
    vk::Buffer buffer = {};
    vk::DescriptorType descriptorType = vk::DescriptorType::eUniformBuffer;
    uint32_t offset   = 0;
    uint32_t range    = 0;
};

struct TransientSamplerBinding final
{
    uint32_t binding    = 0;
    vk::Sampler sampler = {};
    BindType bindType   = BindType::Sampler;
};

struct TransientBindGroupUpdateDesc final
{
    std::vector<TransientTextureBinding> textures;
    std::vector<TransientBufferBinding> buffers;
    std::vector<TransientSamplerBinding> samplers;
};

} // namespace gore::gfx