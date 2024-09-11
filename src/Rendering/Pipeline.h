#pragma once

#include "Prefix.h"

#include "Handle.h"

#include "Graphics/Vulkan/VulkanIncludes.h"
#include "Graphics/Vulkan/VulkanExtensions.h"

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
};

struct ComputePipeline final : Pipeline
{
    using Pipeline::Pipeline;
};

using GraphicsPipelineHandle = Handle<GraphicsPipeline>;
using ComputePipelineHandle  = Handle<ComputePipeline>;
} // namespace gore