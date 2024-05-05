#pragma once

#include <stdint.h>

namespace gore::gfx::geometry
{
enum class ShaderChannel : uint8_t
{
    None     = 0,
    Position = 1 << 0,
    UV0      = 1 << 1,
    UV1      = 1 << 2,
    UV2      = 1 << 3,
    UV3      = 1 << 4,
    Normal   = 1 << 5,
    Tangent  = 1 << 6,
    Color    = 1 << 7,
};

int CalculateShaderChannelsByteStrideSize(uint8_t channels);
} // namespace gore::gfx::geometry