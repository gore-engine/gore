#pragma once

#include "Prefix.h"
#include "Draw.h"

#include <vector>
#include <unordered_map>

namespace gore::renderer
{
union DrawStateMask
{
    struct
    {
        uint32_t shader : 1;
        uint32_t bindgroup0 : 1;
        uint32_t bindgroup1 : 1;
        uint32_t bindgroup2 : 1;
        uint32_t indexBuffer : 1;
        uint32_t vertexBuffer : 1;
        uint32_t dynamicBuffer : 1;
        uint32_t indexOffset : 1;
        uint32_t vertexOffset : 1;
        uint32_t instanceOffset : 1;
        uint32_t instanceCount : 1;
        uint32_t dynamicBufferOffset : 1;
        uint32_t indexCount : 1;

        uint32_t pack : 19;
    };

    uint32_t mask;
};
static_assert(sizeof(DrawStateMask) == 4, "DrawStateMask should be 4 bytes");

struct DrawStream final
{
    std::vector<uint8_t> data;
};

void CreateDrawStreamFromDrawData(const std::vector<Draw>& drawData, DrawStream& drawStream);
void ScheduleDrawStream(RenderContext& renderContext, DrawStream& drawStream, vk::CommandBuffer commandBuffer, GraphicsPipelineHandle overridePipeline = {});
} // namespace gore::renderer   