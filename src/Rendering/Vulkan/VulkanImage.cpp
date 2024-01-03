#include "Prefix.h"

#include "VulkanImage.h"
#include "VulkanDevice.h"

namespace gore
{

VulkanImage::VulkanImage(VulkanDevice* device, VkImage image, VkFormat format) :
    m_Device(device),
    m_Image(image),
    m_Format(format),
    m_OwnsImage(false),
//    m_ShaderResourceView(VK_NULL_HANDLE),
//    m_DepthStencilView(VK_NULL_HANDLE),
//    m_UnorderedAccessView(VK_NULL_HANDLE),
    m_RenderTargetView(VK_NULL_HANDLE)
{
    // TODO: more views and improve this
    VkImageViewCreateInfo viewCreateInfo{
        .sType      = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext      = nullptr,
        .flags      = 0,
        .image      = image,
        .viewType   = VK_IMAGE_VIEW_TYPE_2D,
        .format     = m_Format,
        .components = {
                       .r = VK_COMPONENT_SWIZZLE_R,
                       .g = VK_COMPONENT_SWIZZLE_G,
                       .b = VK_COMPONENT_SWIZZLE_B,
                       .a = VK_COMPONENT_SWIZZLE_A,
                       },
        .subresourceRange = {
                       .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                       .baseMipLevel   = 0,
                       .levelCount     = 1,
                       .baseArrayLayer = 0,
                       .layerCount     = 1,
                       },
    };

    VkResult res = m_Device->API.vkCreateImageView(m_Device->Get(), &viewCreateInfo, nullptr, &m_RenderTargetView);
    VK_CHECK_RESULT(res);
}

VulkanImage::~VulkanImage()
{
    if (m_RenderTargetView != VK_NULL_HANDLE)
    {
        m_Device->API.vkDestroyImageView(m_Device->Get(), m_RenderTargetView, nullptr);
    }

    if (!m_OwnsImage)
        return;
}

} // namespace gore
