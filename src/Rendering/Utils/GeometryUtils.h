#pragma once

#include <stdint.h>

namespace gore::gfx::geometry
{
enum class IndexType : uint8_t
{
    None   = 0,
    UInt16 = 1,
    UInt32 = 2,
};

inline int GetIndexTypeSize(IndexType indexType)
{
    switch (indexType)
    {
        case IndexType::UInt16:
            return sizeof(uint16_t);
        case IndexType::UInt32:
            return sizeof(uint32_t);
        default:
            return 0;
    }
}

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

int CalculateVertexBufferSize(uint8_t channels, uint32_t vertexCount);
int CalculateIndexBufferSize(IndexType indexType, uint32_t indexCount);
} // namespace gore::gfx::geometry