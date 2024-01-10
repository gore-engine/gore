#pragma once

#include "Utilities/Defines.h"

#include "Graphics/Vulkan/VulkanIncludes.h"
#include "Graphics/Vulkan/VulkanExtensions.h"

#include <vector>

namespace gore
{

class App;

namespace gfx
{

class PhysicalDevice;

class Instance
{
public:
    Instance();
    explicit Instance(App* app);
    Instance(Instance&& other) noexcept;
    ~Instance();

    [[nodiscard]] const vk::raii::Instance& Get() const { return m_Instance; }
    [[nodiscard]] App* GetApp() const { return m_App; }
    [[nodiscard]] uint32_t Version() const { return m_ApiVersion; }

    [[nodiscard]] bool HasExtension(VulkanInstanceExtension extension) const;

    [[nodiscard]] std::vector<PhysicalDevice> GetPhysicalDevices() const;

private:
    App* m_App;

    vk::raii::Context m_Context;
    uint32_t m_ApiVersion;
    vk::raii::Instance m_Instance;
    VulkanInstanceExtensionBitset m_EnabledInstanceExtensions;
    bool m_ValidationEnabled;
};

} // namespace gfx

} // namespace gore
