#include "Prefix.h"

#include "VulkanImage.h"

namespace gore
{

VulkanImage::VulkanImage(VulkanDevice* device, VkImage image) :
    m_Device(device),
    m_Image(image),
    m_OwnsImage(false)
{
}

VulkanImage::~VulkanImage()
{
    if (!m_OwnsImage) return;
}


}