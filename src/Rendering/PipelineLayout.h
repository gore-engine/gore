#pragma once
#include "Prefix.h"

#include "Graphics/Vulkan/VulkanIncludes.h"
#include "Graphics/Vulkan/VulkanExtensions.h"

namespace gore
{
struct PipelineLayout
{
    vk::PipelineLayout layout = VK_NULL_HANDLE;
};
} // namespace gore