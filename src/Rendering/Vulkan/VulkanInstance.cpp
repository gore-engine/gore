#include "Prefix.h"

#include "VulkanInstance.h"
#include "VulkanExtensions.h"
#include "VulkanDevice.h"

#include "Core/Log.h"

#include <unordered_set>

namespace gore
{

VulkanInstance::VulkanInstance(App* app) :
    m_App(app),
    m_Instance(VK_NULL_HANDLE),
    m_EnabledExtensions()
{
    VkResult res = volkInitialize();

    VK_CHECK_RESULT(res);
    if (res != VK_SUCCESS)
    {
        LOG(FATAL, "Failed to load vulkan library\n");
        return;
    }

    m_EnabledExtensions.reset();

    // Enumerate and build enabled instance layers
    std::vector<VkLayerProperties> instanceLayerProperties = GetAvailableInstanceLayers();

#if ENGINE_DEBUG
    const bool enableValidation = true;
#else
    const bool enableValidation = false;
#endif

    std::vector<const char*> enabledLayerNames;
    bool validationEnabled = GetEnabledInstanceLayers(enableValidation,
                                                      instanceLayerProperties,
                                                      enabledLayerNames);

    // Enumerate available and build instance extensions
    std::vector<VkExtensionProperties> instanceExtensionProperties = GetAvailableInstanceExtensions(enabledLayerNames);

    std::vector<const char*> enabledInstanceExtensions = GetEnabledInstanceExtensions(m_EnabledExtensions,
                                                                                      instanceLayerProperties,
                                                                                      instanceExtensionProperties);

    // Create VkInstance
    VkApplicationInfo appInfo{
        .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext              = VK_NULL_HANDLE,
        .pApplicationName   = "GORE",
        .applicationVersion = 1,
        .pEngineName        = "GORE",
        .engineVersion      = 1,
        .apiVersion         = volkGetInstanceVersion(),
    };

    VkInstanceCreateInfo instanceCreateInfo{
        .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext                   = VK_NULL_HANDLE,
        .flags                   = 0,
        .pApplicationInfo        = &appInfo,
        .enabledLayerCount       = static_cast<uint32_t>(enabledLayerNames.size()),
        .ppEnabledLayerNames     = enabledLayerNames.empty() ? VK_NULL_HANDLE : enabledLayerNames.data(),
        .enabledExtensionCount   = static_cast<uint32_t>(enabledInstanceExtensions.size()),
        .ppEnabledExtensionNames = enabledInstanceExtensions.empty() ? VK_NULL_HANDLE : enabledInstanceExtensions.data(),
    };

#if defined(VK_EXT_validation_features) && (VK_EXT_VALIDATION_FEATURES_SPEC_VERSION + 0) >= 4

    // todo: make this a setting
    const bool enableShaderPrintf = true;

    VkValidationFeatureEnableEXT enabledValidationFeatures[1];
    VkValidationFeatureDisableEXT disabledValidationFeatures[1];
    VkValidationFeaturesEXT validationFeatures{
        .sType                          = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT,
        .pNext                          = VK_NULL_HANDLE,
        .enabledValidationFeatureCount  = 0,
        .pEnabledValidationFeatures     = enabledValidationFeatures,
        .disabledValidationFeatureCount = 0,
        .pDisabledValidationFeatures    = disabledValidationFeatures,
    };

    if (validationEnabled && enableShaderPrintf)
    {
        enabledValidationFeatures[0]                      = VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT;
        validationFeatures.enabledValidationFeatureCount  = 1;
        disabledValidationFeatures[0]                     = VK_VALIDATION_FEATURE_DISABLE_ALL_EXT;
        validationFeatures.disabledValidationFeatureCount = 1;
    }
    if (validationEnabled)
        instanceCreateInfo.pNext = &validationFeatures;
#endif

#if defined(VK_KHR_portability_enumeration)
    if (HasExtension(VulkanInstanceExtension::kVK_KHR_portability_enumeration))
        instanceCreateInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

    res = vkCreateInstance(&instanceCreateInfo, VK_NULL_HANDLE, &m_Instance);
    VK_CHECK_RESULT(res);

    if (res != VK_SUCCESS)
    {
        LOG(FATAL, "Failed to create Vulkan instance.\n");
        return;
    }

    LOG_STREAM(INFO) << "Created Vulkan instance version "
                     << VK_API_VERSION_MAJOR(appInfo.apiVersion) << "."
                     << VK_API_VERSION_MINOR(appInfo.apiVersion) << "."
                     << VK_API_VERSION_PATCH(appInfo.apiVersion) << std::endl;

    volkLoadInstanceOnly(m_Instance);
}

VulkanInstance::~VulkanInstance()
{
    if (m_Instance)
    {
        vkDestroyInstance(m_Instance, VK_NULL_HANDLE);
        LOG(INFO, "Destroyed Vulkan instance\n");
    }
}

bool VulkanInstance::HasExtension(VulkanInstanceExtension extension) const
{
    return m_EnabledExtensions.test(static_cast<size_t>(extension));
}

std::vector<VkLayerProperties> VulkanInstance::GetAvailableInstanceLayers()
{
    std::vector<VkLayerProperties> instanceLayerProperties;
    uint32_t instanceLayerPropertyCount = 0;

    VkResult res = vkEnumerateInstanceLayerProperties(&instanceLayerPropertyCount, VK_NULL_HANDLE);
    VK_CHECK_RESULT(res);
    if (instanceLayerPropertyCount > 0)
    {
        instanceLayerProperties.resize(instanceLayerPropertyCount);
        res = vkEnumerateInstanceLayerProperties(&instanceLayerPropertyCount, &instanceLayerProperties[0]);
        VK_CHECK_RESULT(res);
    }

    return instanceLayerProperties;
}

bool VulkanInstance::GetEnabledInstanceLayers(bool enableValidation,
                                              const std::vector<VkLayerProperties>& availableLayers,
                                              std::vector<const char*>& enabledLayers)
{
    std::unordered_set<std::string> installedLayerNames;
    installedLayerNames.reserve(availableLayers.size());
    for (const auto& property : availableLayers)
    {
        installedLayerNames.insert(property.layerName);
    }

    auto IsLayerInstalled = [&](const char* layerName)
    { return installedLayerNames.find(layerName) != installedLayerNames.end(); };

    // Validation Layer
    bool validation = enableValidation;

    if (validation)
    {
        // TODO: For now we only use standard validation layer. Add more if needed.
        const char* khronosValidationLayerName = "VK_LAYER_KHRONOS_validation";
        if (IsLayerInstalled(khronosValidationLayerName))
        {
            enabledLayers.push_back(khronosValidationLayerName);
        }
    }

    // Other layers can follow here

    if (!enabledLayers.empty())
    {
        LOG(DEBUG, "Enabled instance layers:\n");
        for (const auto& layer : enabledLayers)
        {
            LOG(DEBUG, "  %s\n", layer);
        }
    }

    return validation;
}

std::vector<VulkanPhysicalDevice> VulkanInstance::GetPhysicalDevices()
{
    std::vector<VulkanPhysicalDevice> physicalDevices;

    uint32_t count = 0;
    VkResult res   = vkEnumeratePhysicalDevices(m_Instance, &count, VK_NULL_HANDLE);

    VK_CHECK_RESULT(res);
    if (res != VK_SUCCESS)
    {
        LOG(ERROR, "Failed to enumerate physical devices.\n");
        return physicalDevices;
    }

    if (count == 0)
    {
        LOG(ERROR, "No physical devices found.\n");
        return physicalDevices;
    }

    std::vector<VkPhysicalDevice> vkPhysicalDevices(count);
    res = vkEnumeratePhysicalDevices(m_Instance, &count, vkPhysicalDevices.data());

    VK_CHECK_RESULT(res);
    if (res != VK_SUCCESS)
    {
        LOG(ERROR, "Failed to enumerate physical devices.\n");
        return physicalDevices;
    }

    physicalDevices.reserve(count);

    for (int i = 0; i < count; ++i)
    {
        const auto& vkPhysicalDevice = vkPhysicalDevices[i];

        VulkanPhysicalDevice physicalDevice(this, i, vkPhysicalDevice);

        physicalDevices.push_back(physicalDevice);

        physicalDevice.LogInfo();
    }

    return physicalDevices;
}

} // namespace gore