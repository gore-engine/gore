#pragma once

#include "Prefix.h"

namespace gore
{
enum class GraphicsFormat
{
    Undefined,
    // Common Render Target Formats
    R11G11B10_UNORM,

    // Common Texture Formats
    BGRA8_SRGB,
    RGBA8_SRGB,
    RGBA8_UNORM,
    RGB8_SRGB,

    // Common Vertex Format
    RGB32_FLOAT, // postion
    RGB16_FLOAT, // normal, tangent
    RGB8_UNORM,  // color
    RG16_FLOAT,  // uv
    RG32_FLOAT,  // uv for large texture sample
    R32_UINT,    // indices
    R16_UINT,    // indices
    R8_UINT,     // indices for small mesh

    // AttributePosition = RGB32_FLOAT,
    // AttributeNormal   = RGB16_FLOAT,
    // AttributeTangent  = RGB16_FLOAT,
    // AttributeColor    = RGB8_UNORM,
    // AttributeUV0      = RG16_FLOAT,

    // Depth Stencil Format
    D32_FLOAT, // depth 24 bits, stencil 8 bits
    D32_FLOAT_S8_UINT,
    Count
};

} // namespace gore