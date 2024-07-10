#pragma once
#include "Prefix.h"

#include "Buffer.h"

#include "Graphics/Vulkan/VulkanIncludes.h"

namespace gore::gfx
{
struct DynamicBufferDesc
{
    const char* debugName = "Noname DynamicBuffer";
    Handle<Buffer> buffer = {};
    uint32_t offset       = 0;
    uint32_t range        = 0;
};

struct DynamicBuffer
{
    vk::DescriptorSet set          = {};
    vk::DescriptorSetLayout layout = {};
    uint32_t offset                = 0;
};

using DynamicBufferHandle = Handle<DynamicBuffer>;

} // namespace gore::gfx