#pragma once

#include "VulkanIncludes.h"

namespace gore
{
    bool InitializeVulkanInstance(VkInstance* instance);
    bool ShutdownVulkanInstance(VkInstance instance);
}
