#pragma once

#include "Prefix.h"

#include "Handle.h"

#include "Graphics/Vulkan/VulkanIncludes.h"
#include "Graphics/Vulkan/VulkanExtensions.h"

#include "Graphics/VulkanBuffer.h"

namespace gore
{
struct Buffer
{
    Buffer(gfx::VulkanBuffer&& inBuffer) :
        vkBuffer(std::move(inBuffer))
    {
    }

    gfx::VulkanBuffer vkBuffer;
};

struct BindGroup
{
};

struct ShaderModule
{
    ShaderModule(vk::raii::ShaderModule&& inSM) :
        sm(std::move(inSM))
    {
    }

    vk::raii::ShaderModule sm;
};

using BufferHandle       = Handle<Buffer>;
using ShaderModuleHandle = Handle<ShaderModule>;
} // namespace gore