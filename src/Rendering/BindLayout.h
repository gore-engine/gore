#pragma once

#include "Vulkan/VulkanIncludes.h"

namespace gore
{
struct BindLayout final
{
    const char* debugName = "Noname BindLayout";
    VkDescriptorSetLayout descriptorSetLayout;
};
} // namespace gore