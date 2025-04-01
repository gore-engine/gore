#pragma once

#include "Graphics/Vulkan/VulkanIncludes.h"

namespace gore::gfx
{
struct DescriptorPoolHolder final
{
    vk::DescriptorPool pool = {};

    operator vk::DescriptorPool() const
    {
        return pool;
    }

    operator vk::DescriptorPool&()
    {
        return pool;
    }
};
} // namespace gore::gfx