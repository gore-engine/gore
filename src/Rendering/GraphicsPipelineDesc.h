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

struct MultisampleState final
{
    uint32_t sampleCount = 1;
};

struct DepthStencilState final
{
    CompareOp depthTest = CompareOp::LessEqual;
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

struct BlendState final
{
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