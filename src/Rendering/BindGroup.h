#pragma once

#include "Handle.h"

#include <vector>

namespace gore
{
struct BindLayout;

struct BufferBinding final
{
    BufferHandle handle = {};
    uint32_t byteOffset = 0;
};

struct BindGroup final
{
    const char* debugName               = "Noname BindGroup";
    const BindLayout* bindLayout        = nullptr;
    std::vector<TextureHandle> textures = {};
    std::vector<BufferBinding> buffers  = {};
};
} // namespace gore