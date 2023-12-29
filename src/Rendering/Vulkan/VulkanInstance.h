#pragma once

#include "VulkanIncludes.h"
#include "VulkanExtensions.h"

namespace gore
{

class App;
struct VulkanPhysicalDevice;

class VulkanInstance
{
public:
    VulkanInstance(App* app);
    ~VulkanInstance();

    bool Initialize();
    bool Shutdown();

    [[nodiscard]] VkInstance Get() const { return m_Instance; }
    [[nodiscard]] App* GetApp() const { return m_App; }
    [[nodiscard]] bool HasExtension(VulkanInstanceExtension extension) const;

    std::vector<VulkanPhysicalDevice> GetPhysicalDevices();

private:
    App* m_App;

    VkInstance m_Instance;
    VulkanInstanceExtensionBitset m_EnabledExtensions;

    // Instance Layers
    static std::vector<VkLayerProperties> GetAvailableInstanceLayers();
    // Returns: if validation layer is enabled
    bool GetEnabledInstanceLayers(bool enableValidation,
                                  const std::vector<VkLayerProperties>& availableLayers,
                                  std::vector<const char*>& enabledLayers);
};

} // namespace gore
