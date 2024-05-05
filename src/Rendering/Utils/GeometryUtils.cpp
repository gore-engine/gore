#include "GeometryUtils.h"

#include "Math/Types.h"

int gore::gfx::geometry::CalculateShaderChannelsByteStrideSize(uint8_t channels)
{
    const int k_PositionSize = sizeof(Vector3);
    const int k_UVSize       = sizeof(Vector2);
    const int k_NormalSize   = sizeof(Vector3);
    const int k_TangentSize  = sizeof(Vector3);
    const int k_ColorSize    = sizeof(uint8_t) * 4;

    int byteStride = 0;
    if (channels & static_cast<uint8_t>(ShaderChannel::Position))
    {
        byteStride += k_PositionSize;
    }

    if (channels & static_cast<uint8_t>(ShaderChannel::UV0))
    {
        byteStride += k_UVSize;
    }

    if (channels & static_cast<uint8_t>(ShaderChannel::UV1))
    {
        byteStride += k_UVSize;
    }

    if (channels & static_cast<uint8_t>(ShaderChannel::UV2))
    {
        byteStride += k_UVSize;
    }

    if (channels & static_cast<uint8_t>(ShaderChannel::UV3))
    {
        byteStride += k_UVSize;
    }

    if (channels & static_cast<uint8_t>(ShaderChannel::Normal))
    {
        byteStride += k_NormalSize;
    }

    if (channels & static_cast<uint8_t>(ShaderChannel::Tangent))
    {
        byteStride += k_TangentSize;
    }

    if (channels & static_cast<uint8_t>(ShaderChannel::Color))
    {
        byteStride += k_ColorSize;
    }

    return byteStride;
}