#pragma once
#include "Graphics/Vulkan/VulkanIncludes.h"

namespace gore::gfx
{
class Instance;
class Device;

struct GraphicsCaps
{
    size_t minUniformBufferOffsetAlignment = 0;

    bool supportsBindless = false;
};

void InitVulkanGraphicsCaps(GraphicsCaps& caps, Instance& instance, Device& device);
} // namespace gore::gfx