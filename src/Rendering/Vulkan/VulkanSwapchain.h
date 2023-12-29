#pragma once

#include "VulkanIncludes.h"

#include <vector>

namespace gore
{

class VulkanSurface;

class VulkanSwapchain
{
public:
    VulkanSwapchain(VulkanSurface* surface, uint32_t imageCount);
    ~VulkanSwapchain();

    [[nodiscard]] VkSwapchainKHR Get() const { return m_Swapchain; }
    [[nodiscard]] VkSurfaceFormatKHR GetFormat() const { return m_Format; }
    [[nodiscard]] VkFormat GetImageFormat() const { return m_Format.format; }
    [[nodiscard]] VkColorSpaceKHR GetColorSpace() const { return m_Format.colorSpace; }
    [[nodiscard]] uint32_t GetWidth() const { return m_Width; }
    [[nodiscard]] uint32_t GetHeight() const { return m_Height; }
    [[nodiscard]] uint32_t GetLayers() const { return m_Layers; }
    [[nodiscard]] uint32_t GetImageCount() const { return m_ImageCount; }
    [[nodiscard]] const std::vector<VkImage>& GetImages() const { return m_Images; }
    [[nodiscard]] const std::vector<VkImageView>& GetImageViews() const { return m_ImageViews; }

private:
    VulkanSurface* m_Surface;

    VkSwapchainKHR m_Swapchain;
    VkSurfaceFormatKHR m_Format;
    uint32_t m_Width;
    uint32_t m_Height;
    uint32_t m_Layers;

    uint32_t m_ImageCount;

    std::vector<VkImage> m_Images;
    std::vector<VkImageView> m_ImageViews;

    uint32_t m_CurrentImageIndex;
};

} // namespace gore
