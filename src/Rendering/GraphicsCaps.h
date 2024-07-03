#pragma once
#include "Graphics/Vulkan/VulkanIncludes.h"

namespace gore::gfx
{
struct GraphicsCaps
{
    size_t minUniformBufferOffsetAlignment = 0;
};

void InitVulkanGraphicsCaps(GraphicsCaps& caps, vk::Instance instance, vk::PhysicalDevice physicalDevice);
} // namespace gore::gfx