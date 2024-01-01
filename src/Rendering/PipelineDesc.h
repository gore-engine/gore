#pragma once

enum class TopologyType
{
    Point,
    Line,
    TriangleList,
    // FIXME: Other are deprecated
    Count
};

struct VertexInputDesc final
{
    
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
};