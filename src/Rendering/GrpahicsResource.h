#pragma once

#include "Prefix.h"

#include "Handle.h"

#include "Graphics/Vulkan/VulkanIncludes.h"
#include "Graphics/Vulkan/VulkanExtensions.h"

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

using TextureHandle      = Handle<Texture>;
using BufferHandle       = Handle<Buffer>;
using SamplerHandle      = Handle<Sampler>;
using BindGroupHandle    = Handle<BindGroup>;
} // namespace gore