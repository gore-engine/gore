#pragma once

#include "Graphics/Vulkan/VulkanIncludes.h"
#include "Graphics/Vulkan/VulkanExtensions.h"

#include "GraphicsResourcePrefix.h"

#include <vector>

namespace gore
{

enum class BindType : uint8_t
{
    UniformBuffer,
    StorageBuffer,
    CombinedSampledImage,
    SampledImage,
    StorageImage,
    Sampler,
    Count
};

struct Binding final
{
    uint8_t binding;
    BindType type;
    uint8_t descriptorCount;
    ShaderStage stage;
};

struct BindLayoutCreateInfo final
{
    const char* name              = nullptr;
    std::vector<Binding> bindings = {};
};

struct BindLayout final
{
    /// @brief Vulkan : DescriptorSetLayout
    vk::DescriptorSetLayout layout;
};
} // namespace gore