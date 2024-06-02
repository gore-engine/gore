#pragma once

#include "Prefix.h"
#include "GraphicsFormat.h"
#include "BindGroup.h"
#include "BindLayout.h"
#include "PipelineLayout.h"

#include "Math/Rect.h"

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

struct ShaderModuleDesc final
{
    const char* debugName = "Noname ShaderModuleDesc";
    uint8_t* byteCode     = nullptr;
    uint32_t byteSize     = 0;
    const char* entryFunc = "main";
};

struct ShaderBinding final
{
    uint8_t* byteCode;
    uint32_t byteSize;
    const char* entryFunc;
};

struct InputAssemblyState final
{
    TopologyType topology : 7       = TopologyType::TriangleList;
    bool primitiveRestartEnable : 1 = false;
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
    uint32_t sampleMask          = ~0u;
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

struct StencilOpState final
{
    StencilOp failOp      = StencilOp::Replace;
    StencilOp passOp      = StencilOp::Replace;
    StencilOp depthFailOp = StencilOp::Replace;
    CompareOp compareOp   = CompareOp::Never;
};

struct DepthStencilState final
{
    // reverse depth buffer
    bool depthTestEnable       = true;
    bool depthWriteEnable      = true;
    CompareOp depthTestOp      = CompareOp::GreaterEqual;
    bool depthBoundsTestEnable = false;
    bool stencilTestEnable     = false;
    StencilOpState front       = StencilOpState();
    StencilOpState back        = StencilOpState();
    float minDepthBounds       = 0.0f;
    float maxDepthBounds       = 1.0f;
};

struct RasterizationState final
{
    bool depthClamp : 1            = false;
    bool rasterizerDiscard : 1     = false;
    bool frontCounterClockwise : 1 = false;
    bool depthBiasEnable : 1       = false;
    CullMode cullMode : 4          = CullMode::Back;
    PolygonMode polygonMode : 8    = PolygonMode::Fill;
};

static_assert(sizeof(RasterizationState) == 2, "RasterizationState is too big");

struct ViewPort final
{
    float x;
    float y;
    float width;
    float height;
    float minDepth;
    float maxDepth;
};

struct ViewPortState final
{
    int count           = 1;
    ViewPort* viewPorts = nullptr;
};

struct ScissorState final
{
    int count      = 1;
    Rect* scissors = nullptr;
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
    Add             = 0,
    Subtract        = 1,
    ReverseSubtract = 2,
    Min             = 3,
    Max             = 4
};

enum class ColorComponent : uint8_t
{
    R = 1 << 0,
    G = 1 << 1,
    B = 1 << 2,
    A = 1 << 3
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

struct ColorAttachmentBlendState final
{
    bool enable                   = false;
    LogicOp logicOp               = LogicOp::Clear;
    BlendFactor srcColorFactor    = BlendFactor::One;
    BlendFactor dstColorFactor    = BlendFactor::Zero;
    BlendOp colorBlendOp          = BlendOp::Add;
    BlendFactor srcAlphaFactor    = BlendFactor::One;
    BlendFactor dstAlphaFactor    = BlendFactor::Zero;
    BlendOp alphaBlendOp          = BlendOp::Add;
    ColorComponent colorWriteMask = ColorComponent::R | ColorComponent::G | ColorComponent::B | ColorComponent::A;
};

struct BlendState final
{
    bool enable                                        = false;
    LogicOp logicOp                                    = LogicOp::Clear;
    std::vector<ColorAttachmentBlendState> attachments = {ColorAttachmentBlendState()};
};

struct GraphicsPipelineDesc final
{
    const char* debugName = "Noname Graphics GraphicsPipeline";

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
    // ShaderBinding CS;

    // Amplification shader
    ShaderBinding AS;
    // Mesh shader
    ShaderBinding MS;

    std::vector<GraphicsFormat> colorFormats = {};
    GraphicsFormat depthFormat               = GraphicsFormat::Undefined;
    GraphicsFormat stencilFormat             = GraphicsFormat::Undefined;

    std::vector<BindGroupHandle> bindGroups;

    std::vector<VertexBufferBinding> vertexBufferBindings;

    std::vector<BindLayout> bindLayouts;

    InputAssemblyState assemblyState;

    ViewPortState viewPortState = {};
    ScissorState scissorState   = {};

    MultisampleState multisampleState   = {};
    DepthStencilState depthStencilState = {};
    RasterizationState rasterizeState   = {};
    BlendState blendState               = {};

    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkRenderPass renderPass         = VK_NULL_HANDLE;
    uint32_t subpassIndex           = 0;

    bool UseDynamicRendering() const
    {
        return depthFormat != GraphicsFormat::Undefined || stencilFormat != GraphicsFormat::Undefined || colorFormats.size() > 0;
    }
};
} // namespace gore