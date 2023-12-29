#pragma once

#include "VulkanIncludes.h"

namespace gore
{

class VulkanDevice;

class VulkanSurface
{
public:
    VulkanSurface(VulkanDevice* device, void* nativeWindowHandle);
    ~VulkanSurface();

    [[nodiscard]] VkSurfaceKHR Get() const { return m_Surface; }

private:
    VulkanDevice* m_Device;

    VkSurfaceKHR m_Surface;
};

} // namespace gore
