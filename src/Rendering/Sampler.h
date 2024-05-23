#pragma once

#include "Prefix.h"
#include "Handle.h"

#include "Graphics/Vulkan/VulkanIncludes.h"

namespace gore::gfx
{
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