#pragma once

#include "Prefix.h"

#include "Handle.h"

#include "Graphics/Vulkan/VulkanIncludes.h"
#include "Graphics/Vulkan/VulkanExtensions.h"

#include "Graphics/VulkanBuffer.h"

namespace gore
{
struct Texture
{
};

struct Buffer
{
    Buffer(gfx::VulkanBuffer&& inBuffer) :
        vkBuffer(std::move(inBuffer))
    {
    }

    gfx::VulkanBuffer vkBuffer;
};

struct Sampler
{
};

struct BindGroup
{
};

struct ShaderModule
{
    ShaderModule(vk::raii::ShaderModule&& inSM) :
        sm(std::move(inSM))
    {
    }

    vk::raii::ShaderModule sm;
};

using TextureHandle      = Handle<Texture>;
using BufferHandle       = Handle<Buffer>;
using SamplerHandle      = Handle<Sampler>;
using BindGroupHandle    = Handle<BindGroup>;
using ShaderModuleHandle = Handle<ShaderModule>;
} // namespace gore