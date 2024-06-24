#pragma once

#include "Prefix.h"

#include "Handle.h"

#include "Graphics/Vulkan/VulkanIncludes.h"
#include "Graphics/Vulkan/VulkanExtensions.h"

namespace gore
{

struct ShaderModule
{
    ShaderModule(vk::raii::ShaderModule&& inSM) :
        sm(std::move(inSM))
    {
    }

    vk::raii::ShaderModule sm;
};

using ShaderModuleHandle = Handle<ShaderModule>;
} // namespace gore