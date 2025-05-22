#pragma once

#include "Graphics/Vulkan/VulkanIncludes.h"
#include "Graphics/Vulkan/VulkanExtensions.h"

#include "GraphicsResourcePrefix.h"

#include <vector>

namespace gore ::gfx
{
enum class BindType : uint8_t
{
    UniformBuffer,
    DynamicUniformBuffer,
    StorageBuffer,
    CombinedSampledImage,
    SampledImage,
    StorageImage,
    Sampler,
    Count
};

struct Binding final
{
    uint8_t binding         = 0;
    BindType type           = BindType::UniformBuffer;
    uint16_t descriptorCount = 1;
    ShaderStage stage       = ShaderStage::Vertex;
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
} // namespace gore::gfx