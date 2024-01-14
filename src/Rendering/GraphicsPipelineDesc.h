#pragma once

#include "Prefix.h"
#include "GraphicsFormat.h"
#include "BindGroup.h"

#include <vector>

namespace gore
{
enum class TopologyType
{
    Point,
    Line,
    TriangleList,
    // FIXME: Other are deprecated
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

struct VertxBufferBinding final
{
    uint32_t byteStride;
    std::vector<VertexAttributeDesc> attributes;
};

enum class CompareOp
{
    Never,
    Less,
    Equal,
    LessEqual,
    Greater,
    NotEqual,
    GreaterEqual,
    Always,
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

struct RenderState final
{
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
    std::vector<VertxBufferBinding> vertexBufferBindings;

    MultisampleState multisampleState;
    DepthStencilState depthStencilState;
    RenderState renderState;
    BlendState blendState;
};
} // namespace gore