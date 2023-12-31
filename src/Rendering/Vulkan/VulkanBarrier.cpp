#include "Prefix.h"

#include "VulkanBarrier.h"

namespace gore
{

VkImageLayout ImageLayoutFromResourceState(ResourceState state)
{
    if (HasFlag(state, ResourceState::TransferSource))
        return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

    if (HasFlag(state, ResourceState::TransferDest))
        return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

    if (HasFlag(state, ResourceState::RenderTarget))
        return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    if (HasFlag(state, ResourceState::DepthRead))
        return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

    if (HasFlag(state, ResourceState::DepthWrite))
        return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    if (HasFlag(state, ResourceState::UnorderedAccess))
        return VK_IMAGE_LAYOUT_GENERAL;

    if (HasFlag(state, ResourceState::ShaderResource))
        return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    if (HasFlag(state, ResourceState::Present))
        return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    if (HasFlag(state, ResourceState::Common))
        return VK_IMAGE_LAYOUT_GENERAL;

    return VK_IMAGE_LAYOUT_UNDEFINED;
}

VkAccessFlags AccessFlagsFromResourceState(ResourceState state)
{
    VkAccessFlags result = VK_ACCESS_NONE;

    if (HasFlag(state, ResourceState::TransferSource))
        result |= VK_ACCESS_TRANSFER_READ_BIT;

    if (HasFlag(state, ResourceState::TransferDest))
        result |= VK_ACCESS_TRANSFER_WRITE_BIT;

    if (HasFlag(state, ResourceState::UniformBuffer))
        result |= VK_ACCESS_UNIFORM_READ_BIT;

    if (HasFlag(state, ResourceState::VertexBuffer))
        result |= VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;

    if (HasFlag(state, ResourceState::IndexBuffer))
        result |= VK_ACCESS_INDEX_READ_BIT;

    if (HasFlag(state, ResourceState::UnorderedAccess))
        result |= VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;

    if (HasFlag(state, ResourceState::IndirectArgument))
        result |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT;

    if (HasFlag(state, ResourceState::RenderTarget))
        result |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    if (HasFlag(state, ResourceState::DepthWrite))
        result |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    if (HasFlag(state, ResourceState::DepthRead))
        result |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;

    if (HasFlag(state, ResourceState::ShaderResource))
        result |= VK_ACCESS_SHADER_READ_BIT;

    if (HasFlag(state, ResourceState::Present))
        result |= VK_ACCESS_MEMORY_READ_BIT;

    return result;
}

VkPipelineStageFlags PipelineStageFlagsFromAccessFlags(VkAccessFlags accessFlags, VulkanQueueType queueType)
{
    VkPipelineStageFlags flags = 0;

    switch (queueType)
    {
        case VulkanQueueType::Graphics:
        {
            if ((accessFlags & (VK_ACCESS_INDEX_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT)) != 0)
                flags |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;

            if ((accessFlags & (VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT)) != 0)
            {
                flags |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
                flags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                // if device has geometry shader
                {
                    // flags |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
                }
                // if device has tessellation shaders
                {
                    // flags |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT;
                    // flags |= VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
                }
                flags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
            }

            if ((accessFlags & VK_ACCESS_INPUT_ATTACHMENT_READ_BIT) != 0)
                flags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

            if ((accessFlags & (VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)) != 0)
                flags |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

            if ((accessFlags & (VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)) != 0)
                flags |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;

            break;
        }
        case VulkanQueueType::Transfer:
            return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        case VulkanQueueType::Compute:
        {
            if ((accessFlags & (VK_ACCESS_INDEX_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT)) != 0 ||
                (accessFlags & VK_ACCESS_INPUT_ATTACHMENT_READ_BIT) != 0 ||
                (accessFlags & (VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)) != 0 ||
                (accessFlags & (VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)) != 0)
                return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

            if ((accessFlags & (VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT)) != 0)
                flags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

            break;
        }
    }

    if ((accessFlags & VK_ACCESS_INDIRECT_COMMAND_READ_BIT) != 0)
        flags |= VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;

    if ((accessFlags & (VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT)) != 0)
        flags |= VK_PIPELINE_STAGE_TRANSFER_BIT;

    if ((accessFlags & (VK_ACCESS_HOST_READ_BIT | VK_ACCESS_HOST_WRITE_BIT)) != 0)
        flags |= VK_PIPELINE_STAGE_HOST_BIT;

    if (flags == 0)
        flags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

    return flags;
}

}