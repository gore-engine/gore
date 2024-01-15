#pragma once

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

    // debug report
    vk::raii::DebugReportCallbackEXT m_DebugReportCallback;
    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugReportCallback(
        VkDebugReportFlagsEXT flags,
        VkDebugReportObjectTypeEXT objType,
        uint64_t obj,
        size_t location,
        int32_t code,
        const char* layerPrefix,
        const char* msg,
        void* userData);

    // debug utils
    vk::raii::DebugUtilsMessengerEXT m_DebugUtilsMessenger;
    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugUtilsMessengerCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageTypes,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);
};

} // namespace gfx

} // namespace gore
