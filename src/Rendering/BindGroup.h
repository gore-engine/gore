#pragma once

#include "Handle.h"
#include "GraphicsResource.h"
#include "Texture.h"
#include "BindLayout.h"

#include <vector>

namespace gore::gfx
{

/// @brief Buffer handle with byte offset
struct DynamicBuffer final
{
    BufferHandle handle = {};
    uint32_t byteOffset = 0;
};

struct BindGroupDesc final
{
    const char* debugName               = nullptr;
    const BindLayout* bindLayout        = nullptr;
    std::vector<TextureHandle> textures = {};
    std::vector<DynamicBuffer> buffers  = {};
};

struct BindGroup final
{
    /// @brief Vulkan : DescriptorSet
    vk::DescriptorSet set;
};
} // namespace gore::gfx