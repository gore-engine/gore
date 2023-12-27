#pragma once

#include "VulkanIncludes.h"
#include "VulkanExtensions.h"

namespace gore
{

class VulkanInstance
{
public:
    VulkanInstance();
    ~VulkanInstance();

    bool Initialize();
    bool Shutdown();

    [[nodiscard]] VkInstance GetInstance() const { return m_Instance; }
    [[nodiscard]] bool HasExtension(VulkanInstanceExtension extension) const;

private:
    VkInstance m_Instance;
    VulkanInstanceExtensionBitset m_EnabledExtensions;

    // Instance Layers
    std::vector<VkLayerProperties> GetAvailableInstanceLayers();
    // Returns: if validation layer is enabled
    bool GetEnabledInstanceLayers(bool enableValidation,
                                  const std::vector<VkLayerProperties>& availableLayers,
                                  std::vector<const char*>& enabledLayers);
};

} // namespace gore
