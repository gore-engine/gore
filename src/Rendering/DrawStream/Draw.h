#pragma once

#include "Prefix.h"

#include "Rendering/Pipeline.h"
#include "Rendering/BindGroup.h"
#include "Rendering/Buffer.h"
#include "Rendering/DynamicBuffer.h"


namespace gore::renderer
{
using namespace gore::gfx;

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
} // namespace gore::renderer
