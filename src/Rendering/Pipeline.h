#pragma once

#include "Prefix.h"

#include "Handle.h"

namespace gore
{

struct Pipeline
{
    Pipeline(vk::Pipeline&& inPipeline) :
        pipeline(std::move(inPipeline))
    {
    }

    vk::PipelineLayout layout;
    vk::Pipeline pipeline;
};

struct GraphicsPipeline final : Pipeline
{
    using Pipeline::Pipeline;

    VkRenderPass renderPass;
};

struct ComputePipeline final : Pipeline
{
    using Pipeline::Pipeline;
};

using GraphicsPipelineHandle = Handle<GraphicsPipeline>;
using ComputePipelineHandle  = Handle<ComputePipeline>;
} // namespace gore