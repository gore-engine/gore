#pragma once

#include "Prefix.h"
#include "Math/Vector2.h"
#include "Math/Vector3.h"
#include "Math/Vector4.h"
#include <stdint.h>

#include "Rendering/GraphicsFormat.h"

namespace gore::gfx
{

// Default vertex structure
struct Vertex
{
    Vector3 position;
    Vector3 normal;
    Vector2 uv;
};

const Vector3 k_DefaultPosition = Vector3(0.0f, 0.0f, 0.0f);
const Vector2 k_DefaultUV = Vector2(0.0f, 0.0f);
const Vector3 k_DefaultNormal = Vector3(0.0f, 1.0f, 0.0f);
const Vector4 k_DefaultTangent = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
const Vector4 k_DefaultColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);

enum class IndexType : uint8_t
{
    None   = 0,
    UINT8  = 1,
    UInt16 = 2,
    UInt32 = 3,
};

inline IndexType GetIndexTypeByGraphicsFormat(GraphicsFormat format)
{
    switch (format)
    {
        case GraphicsFormat::R8_UINT:
            return IndexType::UINT8;
        case GraphicsFormat::R16_UINT:
            return IndexType::UInt16;
        case GraphicsFormat::R32_UINT:
            return IndexType::UInt32;
        default:
            return IndexType::None;
    }
}

inline int GetIndexTypeSize(IndexType indexType)
{
    switch (indexType)
    {
        case IndexType::UINT8:
            return sizeof(uint8_t);
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

    Default = Position | UV0 | Normal
};

FLAG_ENUM_CLASS(ShaderChannel, uint8_t);

int CalculateShaderChannelsByteStrideSize(uint8_t channels);

int CalculateVertexBufferSize(uint8_t channels, uint32_t vertexCount);
int CalculateIndexBufferSize(IndexType indexType, uint32_t indexCount);

enum class PrimitiveType : uint8_t
{
    Triangle,
    Quad,
    Cube,
    Sphere,
    Cylinder,
    Cone,
    Torus,
    Plane,
    Count
};
} // namespace gore::gfx