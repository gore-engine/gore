#pragma once

#include "Prefix.h"

#include "Graphics/Vulkan/VulkanIncludes.h"
#include "Graphics/Vulkan/VulkanExtensions.h"

namespace gore
{
class DummyVulkan final
{
    NON_COPYABLE(DummyVulkan);

public:
    DummyVulkan();
    ~DummyVulkan();

    void Initialize();
    void Shutdown();

    vk::raii::Context m_Context;
    vk::raii::Instance m_Instance;
    vk::raii::Device m_Device;

    VulkanInstanceExtensionBitset m_EnabledInstanceExtensions;
    uint32_t m_ApiVersion;
};
} // namespace gore