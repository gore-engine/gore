#pragma once

#include "Handle.h"
#include "GraphicsResource.h"
#include "Texture.h"

#include <vector>

namespace gore::gfx
{
struct BindLayout;

/// @brief Buffer handle with byte offset
struct DynamicBuffer final
{
    BufferHandle handle = {};
    uint32_t byteOffset = 0;
};

struct BindGroupDesc final
{
    const char* debugName               = "Noname BindGroupDesc";
    const BindLayout* bindLayout        = nullptr;
    std::vector<TextureHandle> textures = {};
    std::vector<DynamicBuffer> buffers  = {};
};
} // namespace gore::gfx