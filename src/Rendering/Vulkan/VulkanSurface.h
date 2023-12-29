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

private:
    Window* m_Window;

    VulkanDevice* m_Device;

    VkSurfaceKHR m_Surface;
};

} // namespace gore
