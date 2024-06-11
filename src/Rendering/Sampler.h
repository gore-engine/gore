#pragma once

#include "Prefix.h"
#include "Handle.h"

#include "Graphics/Vulkan/VulkanIncludes.h"

namespace gore::gfx
{

enum class SamplerFilter
{
    Nearest,
    Linear,
    Count
};

enum class SamplerAddressMode
{
    Repeat,
    MirroredRepeat,
    ClampToEdge,
    ClampToBorder,
    MirrorClampToEdge,
    Count
};

enum class SamplerMipmapMode
{
    Nearest,
    Linear,
    Count
};

// TODO: Add more options
// Default values are linear, repeat
struct SamplerDesc final
{
    const char* debugName   = "Noname SamplerDesc";
    SamplerFilter minFilter = SamplerFilter::Linear;
    SamplerFilter magFilter = SamplerFilter::Linear;

    SamplerMipmapMode mipmapMode = SamplerMipmapMode::Linear;

    SamplerAddressMode addressModeU = SamplerAddressMode::Repeat;
    SamplerAddressMode addressModeV = SamplerAddressMode::Repeat;
    SamplerAddressMode addressModeW = SamplerAddressMode::Repeat;

    float mipLodBias      = 0.0f;
    CompareOp compareOp   = CompareOp::Never;
    float minLod          = 0.0f;
    float maxLod          = 0.0f;
    float maxAnisotropy   = 1.0f;
    bool anisotropyEnable = false;
    // Color borderColor     = Color::Clear;
};

struct Sampler
{
    vk::Sampler vkSampler = VK_NULL_HANDLE;
};

inline void DestroyVulkanSampler(vk::Device device, vk::Sampler sampler)
{
    device.destroySampler(sampler);
}

using SamplerHandle = Handle<Sampler>;
} // namespace gore::gfx