#pragma once

#include "VulkanIncludes.h"

namespace gore
{

class Window;
class VulkanDevice;

class VulkanSurface
{
public:
    VulkanSurface(VulkanDevice* device, Window* window);
    ~VulkanSurface();

    [[nodiscard]] VkSurfaceKHR Get() const { return m_Surface; }
    [[nodiscard]] VulkanDevice* GetDevice() const { return m_Device; }
    [[nodiscard]] Window* GetWindow() const { return m_Window; }

private:
    Window* m_Window;

    VulkanDevice* m_Device;

    VkSurfaceKHR m_Surface;
};

} // namespace gore
