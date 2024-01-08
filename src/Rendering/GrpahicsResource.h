#pragma once

#include "Prefix.h"

#include "Handle.h"

struct VKShaderModule;

namespace gore
{
struct Texture
{
};

struct Buffer
{
};

struct Sampler
{
};

struct BindGroup
{
};

struct ShaderModule
{
    VKShaderModule* vkShaderModule;
};

using TextureHandle      = Handle<Texture>;
using BufferHandle       = Handle<Buffer>;
using SamplerHandle      = Handle<Sampler>;
using BindGroupHandle    = Handle<BindGroup>;
using ShaderModuleHandle = Handle<ShaderModule>;
} // namespace gore