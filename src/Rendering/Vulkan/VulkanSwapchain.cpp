#include "Prefix.h"

#include "VulkanSwapchain.h"
#include "VulkanDevice.h"
#include "VulkanSurface.h"
#include "VulkanImage.h"
#include "VulkanSynchronization.h"

#include "Windowing/Window.h"

#include <vector>
#include <algorithm>

namespace gore
{

struct SwapchainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

static SwapchainSupportDetails QuerySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
static VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
static VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

VulkanSwapchain::VulkanSwapchain(VulkanSurface* surface, uint32_t imageCount) :
    m_Surface(surface),
    m_Swapchain(VK_NULL_HANDLE),
    m_Format(),
    m_Width(0),
    m_Height(0),
    m_ImageCount(imageCount),
    m_Images(),
    m_RenderFinishedSemaphores(),
    m_ImageAcquiredFences()
{
    Create();
}

VulkanSwapchain::~VulkanSwapchain()
{
    Destroy();
}


void VulkanSwapchain::Create()
{
    VulkanDevice* device            = m_Surface->GetDevice();
    VkPhysicalDevice physicalDevice = device->GetPhysicalDevice().physicalDevice;

    Window* window = m_Surface->GetWindow();
    int width, height;
    window->GetSize(&width, &height);

    SwapchainSupportDetails swapChainSupport = QuerySwapchainSupport(physicalDevice, m_Surface->Get());

    if (swapChainSupport.formats.empty() || swapChainSupport.presentModes.empty())
    {
        LOG(ERROR, "Physical device does not support swapchain\n");
        return;
    }

    VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode     = ChooseSwapPresentMode(swapChainSupport.presentModes); // TODO: vsync

    m_Format.format     = surfaceFormat.format;
    m_Format.colorSpace = surfaceFormat.colorSpace; // TODO: move to outer scope

    m_Width  = std::clamp(static_cast<uint32_t>(width),
                         swapChainSupport.capabilities.minImageExtent.width,
                         swapChainSupport.capabilities.maxImageExtent.width);
    m_Height = std::clamp(static_cast<uint32_t>(height),
                          swapChainSupport.capabilities.minImageExtent.height,
                          swapChainSupport.capabilities.maxImageExtent.height);
    m_Layers = std::min(1u, swapChainSupport.capabilities.maxImageArrayLayers); // TODO: Require layers

    m_ImageCount = std::clamp(m_ImageCount,
                              swapChainSupport.capabilities.minImageCount,
                              swapChainSupport.capabilities.maxImageCount);

    m_CurrentImageIndex = m_ImageCount - 1;

    VkExtent2D extent{
        .width  = m_Width,
        .height = m_Height,
    };

    VkSwapchainCreateInfoKHR createInfo{
        .sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface          = m_Surface->Get(),
        .minImageCount    = m_ImageCount,
        .imageFormat      = m_Format.format,
        .imageColorSpace  = m_Format.colorSpace,
        .imageExtent      = extent,
        .imageArrayLayers = m_Layers,
        .imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, // TODO: Check supported usage and maybe get this from user?
        .preTransform     = swapChainSupport.capabilities.currentTransform,
        .compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode      = presentMode,
        .clipped          = VK_TRUE,
        .oldSwapchain     = VK_NULL_HANDLE,
    };

    VkResult res = device->API.vkCreateSwapchainKHR(device->Get(), &createInfo, VK_NULL_HANDLE, &m_Swapchain);
    VK_CHECK_RESULT(res);
    if (res < 0)
    {
        LOG(ERROR, "Failed to create swapchain\n");
        return;
    }

    std::vector<VkImage> images(m_ImageCount);
    res = device->API.vkGetSwapchainImagesKHR(device->Get(), m_Swapchain, &m_ImageCount, images.data());

    m_Images.resize(m_ImageCount);
    m_RenderFinishedSemaphores.resize(m_ImageCount);
    m_ImageAcquiredFences.resize(m_ImageCount);

    for (uint32_t i = 0; i < m_ImageCount; ++i)
    {
        m_Images[i]                   = new VulkanImage(device, images[i]);
        m_RenderFinishedSemaphores[i] = new VulkanSemaphore(device);
        m_ImageAcquiredFences[i]      = new VulkanFence(device, true);
    }

    AcquireNextImageIndex();

    LOG(DEBUG, "Created Vulkan swapchain with %d images, size %dx%d\n", m_ImageCount, m_Width, m_Height);
}

void VulkanSwapchain::Destroy()
{
    for (uint32_t i = 0; i < m_ImageCount; ++i)
    {
        delete m_Images[i];
        delete m_RenderFinishedSemaphores[i];
        delete m_ImageAcquiredFences[i];
    }

    if (m_Swapchain != VK_NULL_HANDLE)
    {
        m_Surface->GetDevice()->API.vkDestroySwapchainKHR(m_Surface->GetDevice()->Get(), m_Swapchain, VK_NULL_HANDLE);
        LOG(DEBUG, "Destroyed Vulkan swapchain\n");
        m_Swapchain = VK_NULL_HANDLE;
    }
}

void VulkanSwapchain::AcquireNextImageIndex()
{
    VulkanDevice* device = m_Surface->GetDevice();

    VulkanFence* fence = m_ImageAcquiredFences[m_CurrentImageIndex];
    fence->Reset();


    VkResult res = device->API.vkAcquireNextImageKHR(device->Get(), m_Swapchain, UINT64_MAX, VK_NULL_HANDLE, fence->Get(), &m_CurrentImageIndex);
    VK_CHECK_RESULT(res);

    // This will potentially lose some performance.
    // It is designed like this to match the behavior of other APIs.
    fence->Wait();

    RecreateIfRequired();
}

void VulkanSwapchain::Present(const std::vector<VulkanSemaphore*>& waitSemaphores)
{
    VulkanQueue presentQueue = m_Surface->GetDevice()->GetQueue(VulkanQueueType::Present);
    presentQueue.Present(this, waitSemaphores);
}

bool VulkanSwapchain::RecreateIfRequired()
{
    int width, height;
    m_Surface->GetWindow()->GetSize(&width, &height);
    if (m_Width != width || m_Height != height)
    {
        Destroy();
        Create();

        return true;
    }

    return false;
}

SwapchainSupportDetails QuerySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    SwapchainSupportDetails details;

    VkResult res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);
    VK_CHECK_RESULT(res);

    // VkBool32 presentSupport = false;
    // res = vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
    // assert(res == VK_SUCCESS);

    uint32_t surfaceFormatCount;
    res = vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &surfaceFormatCount, VK_NULL_HANDLE);
    VK_CHECK_RESULT(res);

    if (surfaceFormatCount != 0)
    {
        details.formats.resize(surfaceFormatCount);
        res = vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &surfaceFormatCount, details.formats.data());
        VK_CHECK_RESULT(res);
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, VK_NULL_HANDLE);

    if (presentModeCount != 0)
    {
        details.presentModes.resize(presentModeCount);
        res = vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
        VK_CHECK_RESULT(res);
    }

    return details;
}

VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    // TODO: Request Color Space
    // TODO: Request Format
    // for (const auto& availableFormat : availableFormats)
    // {
    //     if (availableFormat.format == requestedFormat)
    //     {
    //         return availableFormat;
    //     }
    // }

    for (const auto& availableFormat : availableFormats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    for (const auto& availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

} // namespace gore
