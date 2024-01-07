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

struct DepthStencilState final
{
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

    std::vector<BindGroupHandle> bindGroups;
    std::vector<VertxBufferBinding> vertexBufferBindings;
};
} // namespace gore