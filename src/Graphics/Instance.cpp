#include "Prefix.h"

#define VMA_IMPLEMENTATION
#include "Instance.h"
#include "Device.h"

#include <vector>

namespace gore::gfx
{

Instance::Instance(App* app) :
    m_App(app),
    m_Context(),
    m_ApiVersion(0),
    m_Instance(nullptr),
    m_ValidationEnabled(false)
{
}

Instance::~Instance()
{
}

void Instance::Create()
{
    // Layers
    std::vector<vk::LayerProperties> layerProperties = m_Context.enumerateInstanceLayerProperties();

    std::vector<const char*> requestedLayers = {
#if ENGINE_DEBUG
        "VK_LAYER_KHRONOS_validation",
#endif
    };

    m_ValidationEnabled = false;
    std::vector<const char*> enabledLayers;
    for (const auto& requestedLayer : requestedLayers)
    {
        auto it = std::find_if(layerProperties.begin(), layerProperties.end(), [&requestedLayer](const vk::LayerProperties& layer)
                               { return strcmp(layer.layerName, requestedLayer) == 0; });

        if (it != layerProperties.end())
        {
            enabledLayers.push_back(it->layerName);
            m_ValidationEnabled = m_ValidationEnabled || strcmp(it->layerName, "VK_LAYER_KHRONOS_validation") == 0;
        }
    }

    if (!enabledLayers.empty())
    {
        LOG(DEBUG, "Enabled instance layers:\n");
        for (const auto& layer : enabledLayers)
        {
            LOG(DEBUG, "  %s\n", layer);
        }
    }

    // Instance Extensions
    std::vector<vk::ExtensionProperties> instanceExtensionProperties = m_Context.enumerateInstanceExtensionProperties();

    for (std::string layer : enabledLayers)
    {
        std::vector<vk::ExtensionProperties> layerExtensionProperties = m_Context.enumerateInstanceExtensionProperties(layer);
        instanceExtensionProperties.insert(instanceExtensionProperties.end(), layerExtensionProperties.begin(), layerExtensionProperties.end());
    }

    m_EnabledInstanceExtensions.set();
#ifndef ENGINE_DEBUG
    m_EnabledInstanceExtensions.reset(static_cast<size_t>(VulkanInstanceExtension::kVK_EXT_debug_report));
    m_EnabledInstanceExtensions.reset(static_cast<size_t>(VulkanInstanceExtension::kVK_EXT_debug_utils));
#endif

    std::vector<const char*> enabledInstanceExtensions = BuildEnabledExtensions<VulkanInstanceExtensionBitset, VulkanInstanceExtension>(instanceExtensionProperties,
                                                                                                                                        m_EnabledInstanceExtensions);

    // Instance Creation
#ifdef VK_API_VERSION_1_1
    m_ApiVersion = m_Context.enumerateInstanceVersion(); // TODO: potential crash
#else
    m_ApiVersion = VK_API_VERSION_1_0;
#endif

    vk::ApplicationInfo appInfo("Gore", 1, "Gore", 1, m_ApiVersion);
    vk::InstanceCreateInfo instanceCreateInfo({}, &appInfo, enabledLayers, enabledInstanceExtensions);

#if defined(VK_EXT_validation_features) && (VK_EXT_VALIDATION_FEATURES_SPEC_VERSION + 0) >= 4

    // todo: make this a setting
    const bool enableShaderPrintf = true;

    std::vector<vk::ValidationFeatureEnableEXT> enabledValidationFeatures;
    std::vector<vk::ValidationFeatureDisableEXT> disabledValidationFeatures;
    vk::ValidationFeaturesEXT validationFeatures;

    if (m_ValidationEnabled)
    {
        if (enableShaderPrintf)
        {
            enabledValidationFeatures.push_back(vk::ValidationFeatureEnableEXT::eDebugPrintf);
        }

        validationFeatures.setEnabledValidationFeatures(enabledValidationFeatures);
        validationFeatures.setDisabledValidationFeatures(disabledValidationFeatures);

        instanceCreateInfo.setPNext(&validationFeatures);
    }
#endif

#if defined(VK_KHR_portability_enumeration)
    if (m_EnabledInstanceExtensions.test(static_cast<size_t>(VulkanInstanceExtension::kVK_KHR_portability_enumeration)))
        instanceCreateInfo.flags |= vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;
#endif

    m_Instance = m_Context.createInstance(instanceCreateInfo);

    LOG_STREAM(INFO) << "Created Vulkan instance version "
                     << VK_API_VERSION_MAJOR(m_ApiVersion) << "."
                     << VK_API_VERSION_MINOR(m_ApiVersion) << "."
                     << VK_API_VERSION_PATCH(m_ApiVersion) << std::endl;
}

void Instance::Destroy()
{
    m_ValidationEnabled = false;
    m_Instance = nullptr;
    m_ApiVersion = 0;
}

bool Instance::HasExtension(VulkanInstanceExtension extension) const
{
    return m_EnabledInstanceExtensions.test(static_cast<size_t>(extension));
}

std::vector<PhysicalDevice> Instance::GetPhysicalDevices() const
{
    std::vector<vk::raii::PhysicalDevice> physicalDevices = m_Instance.enumeratePhysicalDevices();

    std::vector<PhysicalDevice> result;

    for (int i = 0; i < physicalDevices.size(); ++i)
    {
        result.emplace_back(this, i, physicalDevices[i]);
    }

    return result;
}

} // namespace gore::gfx