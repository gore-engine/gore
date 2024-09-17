#pragma once

#include "Prefix.h"

#include "Rendering/Pipeline.h"
#include "Rendering/BindGroup.h"
#include "Rendering/Buffer.h"
#include "Rendering/DynamicBuffer.h"

namespace gore::gfx
{
class MeshRenderer;
}

namespace gore::renderer
{
using namespace gore::gfx;

struct Pass;

// TODO: This is a temporary solution to use fixed pass names, we could use a more generic approach
struct DrawCreateInfo
{
    const char* passName = nullptr;
};

struct Draw
{
    GraphicsPipelineHandle shader     = {};
    BindGroupHandle bindGroup[3]      = {};
    DynamicBufferHandle dynamicBuffer = {};
    BufferHandle vertexBuffer         = {}; // BufferHandle vertexBuffers[3], VertexBuffer could be an array of buffers for instancing or multiple vertex buffers, but for now we only need one
    BufferHandle indexBuffer          = {};
    uint32_t indexCount               = 0;
    uint32_t indexOffset              = 0;
    uint32_t vertexCount              = 0;
    uint32_t vertexOffset             = 0;
    uint32_t instanceCount            = 0;
    uint32_t instanceOffset           = 0;
    uint32_t dynamicBufferOffset      = 0;

    inline uint32_t GetTriangleCount() const
    {
        return indexCount / 3;
    }
};

struct DrawSorter
{
    bool operator()(const Draw& a, const Draw& b) const
    {
        if (a.shader.index() != b.shader.index())
            return a.shader.index() < b.shader.index();

        if (a.bindGroup[0].index() != b.bindGroup[0].index())
            return a.bindGroup[0].index() < b.bindGroup[0].index();

        if (a.bindGroup[1].index() != b.bindGroup[1].index())
            return a.bindGroup[1].index() < b.bindGroup[1].index();

        if (a.bindGroup[2].index() != b.bindGroup[2].index())
            return a.bindGroup[2].index() < b.bindGroup[2].index();

        if (a.dynamicBuffer.index() != b.dynamicBuffer.index())
            return a.dynamicBuffer.index() < b.dynamicBuffer.index();

        if (a.dynamicBufferOffset != b.dynamicBufferOffset)
            return a.dynamicBufferOffset < b.dynamicBufferOffset;

        if (a.vertexBuffer.index() != b.vertexBuffer.index())
            return a.vertexBuffer.index() < b.vertexBuffer.index();

        if (a.indexBuffer.index() != b.indexBuffer.index())
            return a.indexBuffer.index() < b.indexBuffer.index();

        if (a.vertexOffset != b.vertexOffset)
            return a.vertexOffset < b.vertexOffset;

        if (a.indexOffset != b.indexOffset)
            return a.indexOffset < b.indexOffset;

        return false;
    }
};

void PrepareDrawDataAndSort(DrawCreateInfo& info, std::vector<MeshRenderer>& renderers, std::vector<Draw>& sortedDrawData);
bool MatchDrawFilter(const Pass& pass, const DrawCreateInfo& info);
} // namespace gore::renderer
