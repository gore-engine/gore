#pragma once

#include "Core/System.h"

#define VK_NO_PROTOTYPES
#define VK_ENABLE_BETA_EXTENSIONS

#if PLATFORM_WIN
    #define VK_USE_PLATFORM_WIN32_KHR
#elif PLATFORM_ANDROID
    #define VK_USE_PLATFORM_ANDROID_KHR
#elif PLATFORM_LINUX
    #define VK_USE_PLATFORM_XLIB_KHR
#elif PLATFORM_MACOS
    #define VK_USE_PLATFORM_METAL_EXT
#else
    #error Unsupported platform
#endif

#include <vulkan/vulkan.h>

namespace gore
{

class Window;

class RenderSystem final : System
{
public:
    explicit RenderSystem(App* app);
    ~RenderSystem() override;

    RenderSystem(const RenderSystem&)            = delete;
    RenderSystem& operator=(const RenderSystem&) = delete;
    RenderSystem(RenderSystem&&)                 = delete;
    RenderSystem& operator=(RenderSystem&&)      = delete;

    void Initialize() override;
    void Update() override;
    void Shutdown() override;

    void OnResize(Window* window, int width, int height);

private:
    VkInstance m_Instance;
    VkPhysicalDevice m_PhysicalDevice;
    VkDevice m_Device;
};

} // namespace gore
