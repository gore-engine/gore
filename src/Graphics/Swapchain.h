#pragma once

#include "Graphics/Vulkan/VulkanIncludes.h"
#include "Graphics/Vulkan/VulkanExtensions.h"

namespace gore::gfx
{

class Device;

class Swapchain
{
public:
    Swapchain();
    explicit Swapchain(const Device& device, void* nativeWindowHandle, uint32_t imageCount, uint32_t width, uint32_t height);
    Swapchain(Swapchain&& other) noexcept;
    ~Swapchain();

    Swapchain& operator=(Swapchain&& other) noexcept;

    [[nodiscard]] const vk::raii::SwapchainKHR& Get() const { return m_Swapchain; }
    [[nodiscard]] const vk::Extent2D& GetExtent() const { return m_Extent; }
    [[nodiscard]] const vk::SurfaceFormatKHR& GetFormat() const { return m_Format; }
    [[nodiscard]] uint32_t GetImageCount() const { return m_ImageCount; }
    [[nodiscard]] const std::vector<vk::raii::ImageView>& GetImageViews() const { return m_SwapchainImageViews; }
    [[nodiscard]] const std::vector<vk::Image>& GetImages() const { return m_SwapchainImages; }
    [[nodiscard]] uint32_t GetCurrentImageIndex() const { return m_CurrentImageIndex; }

    void Recreate(uint32_t imageCount, uint32_t width, uint32_t height);

    bool Present(const std::vector<vk::Semaphore>& waitSemaphores, const vk::raii::Queue& presentQueue);

private:
    const Device* m_Device;
    void* m_NativeWindowHandle;

    vk::raii::SurfaceKHR m_Surface;
    vk::raii::SwapchainKHR m_Swapchain;
    vk::SurfaceFormatKHR m_Format;
    vk::Extent2D m_Extent;

    uint32_t m_ImageCount;

    std::vector<vk::Image> m_SwapchainImages;
    std::vector<vk::raii::ImageView> m_SwapchainImageViews;
    std::vector<vk::raii::Fence> m_ImageAcquiredFences;

    uint32_t m_CurrentImageIndex;

    void CreateSwapchain();
    void Clear();
    bool NeedRecreate(vk::Result res);
};

} // namespace gore::gfx
