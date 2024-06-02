#pragma once

#include "Handle.h"
#include "GraphicsResource.h"
#include "Texture.h"
#include "BindLayout.h"

#include <vector>

namespace gore::gfx
{

enum class UpdateFrequency : uint8_t
{
    None,
    PerFrame,
    PerBatch,
    // We should never use this, but it's here for completeness
    PerDraw,
    Count
};

/// @brief Buffer handle with byte offset
struct DynamicBuffer final
{
    BufferHandle handle = {};
    uint32_t byteOffset = 0;
};

struct BindGroupDesc final
{
    const char* debugName               = nullptr;
    UpdateFrequency updateFrequency     = UpdateFrequency::None;
    std::vector<TextureHandle> textures = {};
    std::vector<DynamicBuffer> buffers  = {};
    const BindLayout* bindLayout        = nullptr;
};

struct BindGroup final
{
    /// @brief Vulkan : DescriptorSet
    vk::DescriptorSet set;
};

using BindGroupHandle = Handle<BindGroup>;
} // namespace gore::gfx