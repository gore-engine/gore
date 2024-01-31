#pragma once

#include "Prefix.h"

#include "Handle.h"

namespace gore
{

struct Pipeline
{
    VkPipelineLayout layout;
    VkPipeline pipeline;
};

struct GraphicsPipeline final : Pipeline
{
    VkRenderPass renderPass;
};

struct ComputePipeline final : Pipeline
{
};

using GraphicsPipelineHandle = Handle<GraphicsPipeline>;
using ComputePipelineHandle  = Handle<ComputePipeline>;
} // namespace gore