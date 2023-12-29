#include "Prefix.h"

#include "VulkanSurface.h"
#include "VulkanDevice.h"

#if PLATFORM_WIN
    #include "Platform/Windows/Win32Window.h"
#elif PLATFORM_LINUX
    #include "Platform/Linux/X11Window.h"
#elif PLATFORM_MACOS
    #include "Platform/macOS/CocoaWindow.h"
#endif

#ifdef ERROR
    #undef ERROR
#endif

namespace gore
{

VulkanSurface::VulkanSurface(VulkanDevice* device, void* nativeWindowHandle) :
    m_Device(device),
    m_Surface(VK_NULL_HANDLE)
{
    VkInstance instance = m_Device->GetInstance()->Get();
#ifdef VK_KHR_win32_surface

    Win32Window* window = reinterpret_cast<Win32Window*>(nativeWindowHandle);
    VkWin32SurfaceCreateInfoKHR surfaceCreateInfo{
        .sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
        .pNext     = VK_NULL_HANDLE,
        .flags     = 0,
        .hinstance = window->hInstance,
        .hwnd      = window->hWnd,
    };

    VkResult res = vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfo, VK_NULL_HANDLE, &m_Surface);

#elif VK_KHR_xlib_surface

    X11Window* window = reinterpret_cast<X11Window*>(nativeWindowHandle);
    VkXlibSurfaceCreateInfoKHR surfaceCreateInfo{
        .sType  = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
        .pNext  = VK_NULL_HANDLE,
        .flags  = 0,
        .dpy    = window->display,
        .window = window->window,
    };

    VkResult res = vkCreateXlibSurfaceKHR(instance, &surfaceCreateInfo, VK_NULL_HANDLE, &m_Surface);

#elif VK_EXT_metal_surface

    CocoaWindow* window = reinterpret_cast<CocoaWindow*>(nativeWindowHandle);
    VkMetalSurfaceCreateInfoEXT surfaceCreateInfo{
        .sType  = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT,
        .pNext  = VK_NULL_HANDLE,
        .flags  = 0,
        .pLayer = window->layer,
    };

    VkResult res = vkCreateMetalSurfaceEXT(instance, &surfaceCreateInfo, VK_NULL_HANDLE, &m_Surface);

#else
    #error "No supported surface extension available"
#endif

    VK_CHECK_RESULT(res);
    LOG(DEBUG, "Created Vulkan surface\n");
}

VulkanSurface::~VulkanSurface()
{
    if (m_Surface != VK_NULL_HANDLE)
    {
        vkDestroySurfaceKHR(m_Device->GetInstance()->Get(), m_Surface, VK_NULL_HANDLE);
        LOG(DEBUG, "Destroyed Vulkan surface\n");
    }
}

} // namespace gore
