#pragma once

#include "Graphics/Vulkan/VulkanIncludes.h"
#include "Graphics/Vulkan/VulkanExtensions.h"

namespace gore
{
struct BindLayout final
{
    const char* debugName = "Noname BindLayout";
    
    /// @brief Vulkan : DescriptorSetLayout
    VkDescriptorSetLayout layout;
};
} // namespace gore