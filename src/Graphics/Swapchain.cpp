#include "Prefix.h"

#include "Swapchain.h"
#include "Device.h"
#include "Instance.h"

#include "Windowing/Window.h"
#if PLATFORM_WIN
    #include "Platform/Windows/Win32Window.h"
#elif PLATFORM_LINUX
    #include "Platform/Linux/X11Window.h"
#elif PLATFORM_MACOS
    #include "Platform/macOS/CocoaWindow.h"
#endif

namespace gore::gfx
{

Swapchain::Swapchain() :
    m_Device(nullptr),
    m_NativeWindowHandle(nullptr),
    m_Surface(nullptr),
    m_Swapchain(nullptr),
    m_Format(),
    m_Extent(),
    m_ImageCount(0),
    m_SwapchainImages(),
    m_SwapchainImageViews(),
    m_ImageAcquiredFences(),
    m_CurrentImageIndex(0)
{
}

Swapchain::Swapchain(const Device& device, void* nativeWindowHandle, uint32_t imageCount, uint32_t width, uint32_t height) :
    m_Device(&device),
    m_NativeWindowHandle(nativeWindowHandle),
    m_Surface(nullptr),
    m_Swapchain(nullptr),
    m_Format(),
    m_Extent(width, height),
    m_ImageCount(imageCount),
    m_SwapchainImages(),
    m_SwapchainImageViews(),
    m_ImageAcquiredFences(),
    m_CurrentImageIndex(0)
{
    CreateSwapchain();
}

Swapchain::Swapchain(Swapchain&& other) noexcept :
    m_Device(other.m_Device),
    m_NativeWindowHandle(other.m_NativeWindowHandle),
    m_Surface(std::move(other.m_Surface)),
    m_Swapchain(std::move(other.m_Swapchain)),
    m_Format(other.m_Format),
    m_Extent(other.m_Extent),
    m_ImageCount(other.m_ImageCount),
    m_SwapchainImages(std::move(other.m_SwapchainImages)),
    m_SwapchainImageViews(std::move(other.m_SwapchainImageViews)),
    m_ImageAcquiredFences(std::move(other.m_ImageAcquiredFences)),
    m_CurrentImageIndex(other.m_CurrentImageIndex)
{
}

Swapchain::~Swapchain()
{
}

Swapchain& Swapchain::operator=(Swapchain&& other) noexcept
{
    m_Device                   = other.m_Device;
    m_NativeWindowHandle       = other.m_NativeWindowHandle;
    m_Surface                  = std::move(other.m_Surface);
    m_Swapchain                = std::move(other.m_Swapchain);
    m_Format                   = other.m_Format;
    m_Extent                   = other.m_Extent;
    m_ImageCount               = other.m_ImageCount;
    m_SwapchainImages          = std::move(other.m_SwapchainImages);
    m_SwapchainImageViews      = std::move(other.m_SwapchainImageViews);
    m_ImageAcquiredFences      = std::move(other.m_ImageAcquiredFences);
    m_CurrentImageIndex        = other.m_CurrentImageIndex;

    return *this;
}

void Swapchain::CreateSwapchain()
{
#ifdef VK_KHR_win32_surface

    Win32Window* win32Window = reinterpret_cast<Win32Window*>(m_NativeWindowHandle);
    vk::Win32SurfaceCreateInfoKHR surfaceCreateInfo({}, win32Window->hInstance, win32Window->hWnd);
    m_Surface = m_Device->GetInstance()->Get().createWin32SurfaceKHR(surfaceCreateInfo);

#elif VK_KHR_xlib_surface

    X11Window* x11Window = reinterpret_cast<X11Window*>(m_NativeWindowHandle);
    vk::XlibSurfaceCreateInfoKHR surfaceCreateInfo({}, x11Window->display, x11Window->window);
    m_Surface = m_Device->GetInstance()->Get().createXlibSurfaceKHR(surfaceCreateInfo);

#elif VK_EXT_metal_surface

    CocoaWindow* cocoaWindow = reinterpret_cast<CocoaWindow*>(m_NativeWindowHandle);
    vk::MetalSurfaceCreateInfoEXT surfaceCreateInfo({}, cocoaWindow->layer);
    m_Surface = m_Device->GetInstance()->Get().createMetalSurfaceEXT(surfaceCreateInfo);

#else
    #error "No supported surface extension available"
#endif

    const vk::raii::PhysicalDevice& physicalDevice = m_Device->GetPhysicalDevice().Get();

    vk::SurfaceCapabilitiesKHR surfaceCapabilities               = physicalDevice.getSurfaceCapabilitiesKHR(*m_Surface);
    std::vector<vk::SurfaceFormatKHR> surfaceSupportedFormats    = physicalDevice.getSurfaceFormatsKHR(*m_Surface);
    std::vector<vk::PresentModeKHR> surfaceSupportedPresentModes = physicalDevice.getSurfacePresentModesKHR(*m_Surface);

    if (surfaceSupportedFormats.empty() || surfaceSupportedPresentModes.empty())
    {
        LOG(ERROR, "Physical device does not support swapchain\n");
        return;
    }

    m_Format = surfaceSupportedFormats[0];
    for (const auto& format : surfaceSupportedFormats)
    {
        if (format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
        {
            m_Format = format;
            break;
        }
    }

    vk::PresentModeKHR surfacePresentMode = vk::PresentModeKHR::eImmediate;
    for (const auto& presentMode : surfaceSupportedPresentModes)
    {
        if (presentMode == vk::PresentModeKHR::eMailbox)
        {
            surfacePresentMode = presentMode;
            break;
        }
    }

    m_Extent.width  = std::clamp(m_Extent.width,
                                surfaceCapabilities.minImageExtent.width,
                                surfaceCapabilities.maxImageExtent.width);
    m_Extent.height = std::clamp(m_Extent.height,
                                 surfaceCapabilities.minImageExtent.height,
                                 surfaceCapabilities.maxImageExtent.height);
    uint32_t layers = std::min(1u, surfaceCapabilities.maxImageArrayLayers); // TODO: Require layers

    if (surfaceCapabilities.maxImageCount < surfaceCapabilities.minImageCount)
    {
        // fix for some drivers
        std::swap(surfaceCapabilities.maxImageCount, surfaceCapabilities.minImageCount);
    }

    m_ImageCount = std::clamp(m_ImageCount,
                              surfaceCapabilities.minImageCount,
                              surfaceCapabilities.maxImageCount);

    m_CurrentImageIndex = 0;

    vk::SwapchainCreateInfoKHR createInfo({}, *m_Surface, m_ImageCount,
                                          m_Format.format, m_Format.colorSpace,
                                          m_Extent, layers,
                                          vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst, // TODO: Check supported usage and maybe get this from user?
                                          vk::SharingMode::eExclusive,
                                          {}, // queueFamilies
                                          surfaceCapabilities.currentTransform,
                                          vk::CompositeAlphaFlagBitsKHR::eOpaque,
                                          surfacePresentMode,
                                          true,     // clipped
                                          nullptr); // oldSwapchain

    m_Swapchain = m_Device->Get().createSwapchainKHR(createInfo);

    m_SwapchainImages = m_Swapchain.getImages();

    m_SwapchainImageViews.reserve(m_ImageCount);
    m_ImageAcquiredFences.reserve(m_ImageCount);

    for (uint32_t i = 0; i < m_ImageCount; ++i)
    {
        m_Device->SetName(*reinterpret_cast<uint64_t*>(&m_SwapchainImages[i]), vk::Image::objectType, "Swapchain Image " + std::to_string(i));
        vk::ImageViewCreateInfo imageViewCreateInfo({}, m_SwapchainImages[i], vk::ImageViewType::e2D, m_Format.format, {}, {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
        m_SwapchainImageViews.emplace_back(m_Device->Get().createImageView(imageViewCreateInfo));
        m_ImageAcquiredFences.emplace_back(m_Device->Get().createFence({vk::FenceCreateFlagBits::eSignaled}));

        m_Device->SetName(m_SwapchainImageViews[i], "Swapchain Image View " + std::to_string(i));
        m_Device->SetName(m_ImageAcquiredFences[i], "Swapchain Image Acquired Fence " + std::to_string(i));
    }

    LOG(DEBUG, "Created Vulkan swapchain with %d images, size %dx%d\n", m_ImageCount, m_Extent.width, m_Extent.height);
}

void Swapchain::Clear()
{
    // it seems vkDeviceWaitIdle is somewhat buggy on some drivers
    std::vector<vk::Fence> fences;
    fences.reserve(m_ImageAcquiredFences.size());
    for (const auto& fence : m_ImageAcquiredFences)
    {
        fences.emplace_back(*fence);
    }
    vk::Result res = m_Device->Get().waitForFences(fences, true, UINT64_MAX);
    VK_CHECK_RESULT(res);

    m_Device->WaitIdle();

    m_SwapchainImages.clear();
    m_SwapchainImageViews.clear();
    m_ImageAcquiredFences.clear();

    m_Swapchain = nullptr;
    m_Surface = nullptr;
}

void Swapchain::Recreate(uint32_t imageCount, uint32_t width, uint32_t height)
{
    Clear();

    m_Extent = vk::Extent2D(width, height);
    m_ImageCount = imageCount;

    CreateSwapchain();
}

bool Swapchain::Present(const std::vector<vk::Semaphore>& waitSemaphores, const vk::raii::Queue& presentQueue)
{
    bool recreated = false;

    vk::PresentInfoKHR presentInfo(waitSemaphores, *m_Swapchain, m_CurrentImageIndex);

    vk::Result res = vk::Result::eSuccess;

    try
    {
        res = presentQueue.presentKHR(presentInfo);
    }
    catch (vk::OutOfDateKHRError& e)
    {
        res = vk::Result::eErrorOutOfDateKHR;
    }

    if (NeedRecreate(res))
    {
        Recreate(m_ImageCount, m_Extent.width, m_Extent.height);
        recreated = true;
    }

    // we acquire next image here to simulate behaviors of other APIs
    vk::Fence imageAcquiredFence = *m_ImageAcquiredFences[m_CurrentImageIndex];
    m_Device->Get().resetFences({imageAcquiredFence});

    auto acquireResult = m_Swapchain.acquireNextImage(UINT64_MAX, nullptr, imageAcquiredFence);
    res = acquireResult.first;
    m_CurrentImageIndex = acquireResult.second;
    if (NeedRecreate(res))
    {
        Recreate(m_ImageCount, m_Extent.width, m_Extent.height);
        recreated = true;
    }
    else
    {
        // this could potentially hurt performance
        res = m_Device->Get().waitForFences({imageAcquiredFence}, true, UINT64_MAX);
        VK_CHECK_RESULT(res);
    }

    return recreated;
}

bool Swapchain::NeedRecreate(vk::Result res)
{
    return res == vk::Result::eSuboptimalKHR || res == vk::Result::eErrorOutOfDateKHR;
}

} // namespace gore::gfx