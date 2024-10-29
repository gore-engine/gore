#pragma once

#include "Graphics/Vulkan/VulkanIncludes.h"
#include "Graphics/Vulkan/VulkanExtensions.h"

namespace gore
{
struct RenderPass
{
    vk::RenderPass renderPass = VK_NULL_HANDLE;
};
} // namespace gore