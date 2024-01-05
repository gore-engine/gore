#pragma once

#include "Handle.h"

#include <vector>

namespace gore
{
struct BindLayout;

/// @brief Buffer handle with byte offset
struct DynamicBuffer final
{
    BufferHandle handle = {};
    uint32_t byteOffset = 0;
};

struct BindGroup final
{
    const char* debugName               = "Noname BindGroup";
    const BindLayout* bindLayout        = nullptr;
    std::vector<TextureHandle> textures = {};
    std::vector<DynamicBuffer> buffers  = {};
};
} // namespace gore