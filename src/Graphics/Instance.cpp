#include "Prefix.h"

#define VMA_IMPLEMENTATION
#include "Instance.h"
#include "Device.h"

#include "Core/Log.h"

#include <vector>
#include <cstdlib>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace gore::gfx
{


Instance::Instance() :
    m_App(nullptr),
    m_Context(),
    m_ApiVersion(0),
    m_Instance(nullptr),
    m_ValidationEnabled(false),
    m_DebugReportCallback(nullptr),
    m_DebugUtilsMessenger(nullptr),
    m_Loader()
{
}

Instance::Instance(App* app) :
    m_App(app),
    m_Context(),
    m_ApiVersion(0),
    m_Instance(nullptr),
    m_ValidationEnabled(false),
    m_DebugReportCallback(nullptr),
    m_DebugUtilsMessenger(nullptr)
{
    // Layers
    std::vector<vk::LayerProperties> layerProperties = m_Context.enumerateInstanceLayerProperties();

    const std::string validationLayerName("VK_LAYER_KHRONOS_validation");
    // const std::string renderDocLayerName("VK_LAYER_RENDERDOC_Capture");

    std::vector<const char*> requestedLayers = {
#if ENGINE_DEBUG
        validationLayerName.c_str(),
#endif
    };

    m_ValidationEnabled = false;
    std::vector<const char*> enabledLayers;
    for (const auto& layerProperty : layerProperties)
    {
        auto it = std::find_if(requestedLayers.begin(), requestedLayers.end(), [&layerProperty](const char* layerName)
                               { return strcmp(layerName, layerProperty.layerName) == 0; });

        std::string layerName(&*layerProperty.layerName.begin());

        if (it != requestedLayers.end())
        {
            enabledLayers.push_back(*it);
            m_ValidationEnabled = m_ValidationEnabled || layerName == validationLayerName;
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

    // Instance Creation
    m_ApiVersion = VK_API_VERSION_1_3;

    std::vector<const char*> enabledInstanceExtensions = BuildEnabledExtensions<VulkanInstanceExtensionBitset, VulkanInstanceExtension>(instanceExtensionProperties,
                                                                                                                                        m_EnabledInstanceExtensions);

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
    
    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = m_Loader.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
    VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

    VULKAN_HPP_DEFAULT_DISPATCHER.init(*m_Instance);

    // Debug
#if ENGINE_DEBUG
    if (!m_ValidationEnabled)
    {
        return;
    }

    bool hasDebugUtils = false;
    bool hasDebugReport = false;

#if defined(VK_EXT_debug_utils)
    hasDebugUtils = HasExtension(VulkanInstanceExtension::kVK_EXT_debug_utils);
#endif

#if defined(VK_EXT_debug_report)
    hasDebugReport = HasExtension(VulkanInstanceExtension::kVK_EXT_debug_report);
#endif

    if (hasDebugUtils)
    {
        vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo(
            vk::DebugUtilsMessengerCreateFlagsEXT(),
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
            vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
            DebugUtilsMessengerCallback,
            this);

        m_DebugUtilsMessenger = m_Instance.createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfo);
    }
    else if (hasDebugReport)
    {
        vk::DebugReportCallbackCreateInfoEXT debugReportCallbackCreateInfo(
            vk::DebugReportFlagBitsEXT::eInformation |
                vk::DebugReportFlagBitsEXT::eWarning |
                vk::DebugReportFlagBitsEXT::ePerformanceWarning |
                vk::DebugReportFlagBitsEXT::eError |
                vk::DebugReportFlagBitsEXT::eDebug,
            DebugReportCallback,
            this);

        m_DebugReportCallback = m_Instance.createDebugReportCallbackEXT(debugReportCallbackCreateInfo);
    }

#endif
}

Instance::Instance(Instance&& other) noexcept :
    m_App(other.m_App),
    m_Context(std::move(other.m_Context)),
    m_ApiVersion(other.m_ApiVersion),
    m_Instance(std::move(other.m_Instance)),
    m_EnabledInstanceExtensions(other.m_EnabledInstanceExtensions),
    m_ValidationEnabled(other.m_ValidationEnabled),
    m_DebugReportCallback(std::move(other.m_DebugReportCallback)),
    m_DebugUtilsMessenger(std::move(other.m_DebugUtilsMessenger))
{
}

Instance::~Instance()
{
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

VkBool32 Instance::DebugReportCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char* layerPrefix, const char* msg, void* userData)
{
    LogLevel logLevel = LogLevel::DEBUG;
    switch (flags)
    {
    case VK_DEBUG_REPORT_INFORMATION_BIT_EXT:
        logLevel = LogLevel::INFO;
        break;
    case VK_DEBUG_REPORT_WARNING_BIT_EXT:
    case VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT:
        logLevel = LogLevel::WARNING;
        break;
    case VK_DEBUG_REPORT_ERROR_BIT_EXT:
        logLevel = LogLevel::ERROR;
        break;
    case VK_DEBUG_REPORT_DEBUG_BIT_EXT:
        logLevel = LogLevel::DEBUG;
        break;
    default:
        break;
    }

    if (logLevel < Logger::Default().GetLevel())
    {
        return 0;
    }

    bool isPerformanceWarning = flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;

    auto logStream = Logger::Default().StartStream(logLevel, __FILE__, __LINE__);

    if (isPerformanceWarning)
    {
        logStream << "Vulkan Performance: ";
    }
    else
    {
        logStream << "Vulkan Validation: ";
    }

    logStream << msg << std::endl;

    if (objType != VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT)
    {
        logStream << "\t[" << vk::to_string(static_cast<vk::DebugReportObjectTypeEXT>(objType))
                  << "] (0x" << std::hex << obj << std::dec << ")" << std::endl;
    }

    return 0;
}

VkBool32 Instance::DebugUtilsMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
    LogLevel logLevel = LogLevel::DEBUG;
    switch (messageSeverity)
    {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        logLevel = LogLevel::DEBUG;
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        logLevel = LogLevel::INFO;
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        logLevel = LogLevel::WARNING;
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        logLevel = LogLevel::ERROR;
        break;
    default:
        break;
    }

    if (logLevel < Logger::Default().GetLevel())
    {
        return 0;
    }

    auto logStream = Logger::Default().StartStream(logLevel, __FILE__, __LINE__);

    logStream << "Vulkan " << vk::to_string(static_cast<vk::DebugUtilsMessageTypeFlagsEXT>(messageTypes)) << ": "
              << pCallbackData->pMessage;

    if (pCallbackData->pMessageIdName)
    {
        logStream << " <" << pCallbackData->pMessageIdName << ">" << std::endl;
    }
    else
    {
        logStream << std::endl;
    }

    if (pCallbackData->objectCount > 0)
    {
        logStream << "    Objects: ";
        for (uint32_t i = 0; i < pCallbackData->objectCount; ++i)
        {
            if (i != 0)
                logStream << "             ";
            if (pCallbackData->pObjects[i].pObjectName)
                logStream << "\"" << pCallbackData->pObjects[i].pObjectName << "\" ";
            logStream << "[" << vk::to_string(static_cast<vk::ObjectType>(pCallbackData->pObjects[i].objectType))
                      << "] (0x" << std::hex << pCallbackData->pObjects[i].objectHandle << std::dec << ")" << std::endl;
        }
    }

    if (pCallbackData->cmdBufLabelCount > 0)
    {
        logStream << "    Command Buffer Labels: ";
        for (uint32_t i = 0; i < pCallbackData->cmdBufLabelCount; ++i)
        {
            if (i != 0)
                logStream << "                           ";
            logStream << pCallbackData->pCmdBufLabels[i].pLabelName << std::endl;
        }
    }

    if (pCallbackData->queueLabelCount > 0)
    {
        logStream << "    Queue Labels: " << std::endl;
        for (uint32_t i = 0; i < pCallbackData->queueLabelCount; ++i)
        {
            if (i != 0)
                logStream << "                  ";
            logStream << pCallbackData->pQueueLabels[i].pLabelName << std::endl;
        }
    }

    return 0;
}

} // namespace gore::gfx