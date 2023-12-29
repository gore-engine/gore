#pragma once

#include "VulkanIncludes.h"

namespace gore
{

class VulkanDevice;

// stub for now
class VulkanImage
{
public:
    explicit VulkanImage(VulkanDevice* device, VkImage image);
    ~VulkanImage();

    [[nodiscard]] VkImage Get() const { return m_Image; }

private:
    VulkanDevice* m_Device;

    VkImage m_Image;
    bool m_OwnsImage;
};

} // namespace gore
