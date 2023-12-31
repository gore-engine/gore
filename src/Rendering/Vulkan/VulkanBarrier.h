#pragma once

#include "Utilities/Defines.h"

#include "VulkanIncludes.h"
#include "VulkanQueue.h"

namespace gore
{

class VulkanImage;

enum class ResourceState : uint32_t
{
    Common           = 0,
    UniformBuffer    = 1 << 0,
    VertexBuffer     = 1 << 1,
    IndexBuffer      = 1 << 2,
    RenderTarget     = 1 << 3,
    UnorderedAccess  = 1 << 4,
    DepthWrite       = 1 << 5,
    DepthRead        = 1 << 6,
    ShaderResource   = 1 << 7,
    IndirectArgument = 1 << 8,
    TransferDest     = 1 << 9,
    TransferSource   = 1 << 10,
    Present          = 1 << 11,

    GenericRead = UniformBuffer | VertexBuffer | IndexBuffer | ShaderResource | IndirectArgument | TransferSource,

    // TODO: Shading Rate
    // TODO: Raytracing Acceleration Structure
    // TODO: Video Decode/Encode/Process
};
FLAG_ENUM_CLASS(ResourceState, uint32_t)

enum class ResourceBarrierType : uint32_t
{
    Image,
    Buffer,
};

struct VulkanResourceBarrier
{
    ResourceBarrierType type;

    struct
    {
        union
        {
            VulkanImage* image; // Only valid when type == Image
            // VulkanBuffer* buffer; // Only valid when type == Buffer
        };
        // subresource
    };

    ResourceState currentState;
    ResourceState newState;
};

VkImageLayout ImageLayoutFromResourceState(ResourceState state);
VkAccessFlags AccessFlagsFromResourceState(ResourceState state);
VkPipelineStageFlags PipelineStageFlagsFromAccessFlags(VkAccessFlags accessFlags, VulkanQueueType queueType);

} // namespace gore
