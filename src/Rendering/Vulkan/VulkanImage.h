#pragma once

#include "VulkanIncludes.h"

namespace gore
{

class VulkanDevice;

// stub for now
class VulkanImage
{
public:
    explicit VulkanImage(VulkanDevice* device, VkImage image, VkFormat format);
    ~VulkanImage();

    [[nodiscard]] VkImage Get() const { return m_Image; }
    [[nodiscard]] VkFormat GetFormat() const { return m_Format; }

    [[nodiscard]] VkImageView GetRTV() const { return m_RenderTargetView; }

private:
    VulkanDevice* m_Device;

    VkImage m_Image;
    bool m_OwnsImage;

    VkFormat m_Format;

//    VkImageView m_ShaderResourceView;
//    VkImageView m_DepthStencilView;
//    VkImageView m_UnorderedAccessView; // TODO: array
    VkImageView m_RenderTargetView;
};

} // namespace gore
