#include "Prefix.h"

#include "VulkanSwapchain.h"
#include "VulkanDevice.h"
#include "VulkanSurface.h"

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
    m_ImageCount(-1),
    m_Images(),
    m_ImageViews()
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

    m_ImageCount = std::clamp(imageCount,
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


    LOG(DEBUG, "Created Vulkan swapchain with %d images\n", m_ImageCount);
}

VulkanSwapchain::~VulkanSwapchain()
{
    if (m_Swapchain != VK_NULL_HANDLE)
    {
        m_Surface->GetDevice()->API.vkDestroySwapchainKHR(m_Surface->GetDevice()->Get(), m_Swapchain, VK_NULL_HANDLE);
        LOG(DEBUG, "Destroyed Vulkan swapchain\n");
    }
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