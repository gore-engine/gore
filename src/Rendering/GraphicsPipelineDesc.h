#pragma once

#include "Prefix.h"
#include "GraphicsFormat.h"
#include "BindGroup.h"

#include <vector>

namespace gore
{
enum class TopologyType : uint8_t
{
    Point,
    Line,
    TriangleList,
    // FIXME: Other are deprecated
    Count
};

enum class PolygonMode : uint8_t
{
    Fill,
    Line,
    Point,
    Count
};

enum class CullMode : uint8_t
{
    None,
    Front,
    Back,
    FrontAndBack, // TODO: support it?
    Count
};

struct ShaderBinding final
{
    ShaderModuleHandle smHandle;
    const char* entryFunc;
};

struct VertexAttributeDesc final
{
    uint32_t byteOffset;
    GraphicsFormat format;
};

struct VertexBufferBinding final
{
    uint32_t byteStride;
    std::vector<VertexAttributeDesc> attributes;
};

enum class CompareOp : uint8_t
{
    Never,
    Less,
    Equal,
    LessEqual,
    Greater,
    NotEqual,
    GreaterEqual,
    AlwaysTrue,
    Count
};

// 1, 2, 4, 8 enough for everyone
enum class MultiSampleCount : uint8_t
{
    One,
    Two,
    Four,
    Eight,
    Count
};

struct MultisampleState final
{
    MultiSampleCount sampleCount = MultiSampleCount::One;
    bool sampleShadingEnable     = false;
    bool alphaToCoverageEnable   = false;
    bool alphaToOneEnable        = false;
    float minSampleShading       = 0.0f;
    uint32_t sampleMask          = 0;
};

enum class StencilOp : uint8_t
{
    Keep,
    Zero,
    Replace,
    IncrementAndClamp,
    DecrementAndClamp,
    Invert,
    IncrementAndWrap,
    DecrementAndWrap,
    Count
};

struct DepthStencilState final
{
    // reverse depth buffer
    CompareOp depthTest          = CompareOp::GreaterEqual;
    bool depthWrite              = true;
    bool depthBoundsTestEnable   = false;
    bool stencilTestEnable       = false;
    StencilOp frontStencilFailOp = StencilOp::Keep;
    StencilOp frontStencilPassOp = StencilOp::Keep;
};

struct RasterizationState final
{
    bool depthClamp : 1            = false;
    bool rasterizerDiscard : 1     = false;
    bool frontCounterClockwise : 1 = false;
    bool depthBiasEnable : 1       = false;
    CullMode cullMode : 4          = CullMode::None;
    PolygonMode polygonMode : 8    = PolygonMode::Fill;
};

static_assert(sizeof(RasterizationState) == 2, "RasterizationState is too big");

struct ViewPortState final
{
    int x;
    int y;
    int width;
    int height;
};

struct ScissorState final
{
    int x;
    int y;
    int width;
    int height;
};

enum class BlendFactor : uint8_t
{
    Zero,
    One,
    SrcColor,
    OneMinusSrcColor,
    DstColor,
    OneMinusDstColor,
    SrcAlpha,
    OneMinusSrcAlpha,
    DstAlpha,
    OneMinusDstAlpha,
    ConstantColor,
    OneMinusConstantColor,
    ConstantAlpha,
    OneMinusConstantAlpha,
    SrcAlphaSaturate,
    Src1Color,
    OneMinusSrc1Color,
    Src1Alpha,
    OneMinusSrc1Alpha,
    Count
};

// Symplified version of BlendOp
enum class BlendOp : uint8_t
{
    Add,
    Subtract,
    ReverseSubtract,
    Min,
    Max,
    Count
};

enum class ColorComponent : uint8_t
{
    R = 1 << 0,
    G = 1 << 1,
    B = 1 << 2,
    A = 1 << 3,
    Count
};

inline ColorComponent operator|(ColorComponent a, ColorComponent b)
{
    return static_cast<ColorComponent>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

inline ColorComponent operator&(ColorComponent a, ColorComponent b)
{
    return static_cast<ColorComponent>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
}

inline ColorComponent operator~(ColorComponent a)
{
    return static_cast<ColorComponent>(~static_cast<uint8_t>(a));
}

struct BlendState final
{
    bool enable = false;
    BlendOp colorBlendOp = BlendOp::Add;
    BlendFactor srcColorFactor = BlendFactor::One;
    BlendFactor dstColorFactor = BlendFactor::Zero;
    BlendOp alphaBlendOp = BlendOp::Add;
    BlendFactor srcAlphaFactor = BlendFactor::One;
    BlendFactor dstAlphaFactor = BlendFactor::Zero;

    ColorComponent colorWriteMask = ColorComponent::R | ColorComponent::G | ColorComponent::B | ColorComponent::A;
};

struct PipelineDesc final
{
    const char* debugName = "Noname Pipeline";

    TopologyType topology = TopologyType::TriangleList;

    // Vertex shader
    ShaderBinding VS;
    // Pixel shader
    ShaderBinding PS;
    /* Deprecated
    just because I dislike them :(
    and most platforms have overhead of them
    ShaderBinding GS;
    ShaderBinding HS;
    ShaderBinding DS;
    */
    // Compute shader
    ShaderBinding CS;

    // Amplification shader
    ShaderBinding AS;
    // Mesh shader
    ShaderBinding MS;

    std::vector<BindGroupHandle> bindGroups;
    std::vector<VertexBufferBinding> vertexBufferBindings;

    ViewPortState viewPortState;
    ScissorState scissorState;

    MultisampleState multisampleState;
    DepthStencilState depthStencilState;
    RasterizationState renderState;
    BlendState blendState;
};
} // namespace gore