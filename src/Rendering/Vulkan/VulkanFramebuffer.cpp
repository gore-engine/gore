#include "Prefix.h"

#include "VulkanFramebuffer.h"
#include "VulkanRenderPass.h"
#include "VulkanDevice.h"

namespace gore
{

VulkanFramebuffer::VulkanFramebuffer(VulkanDevice* device, VulkanRenderPass* renderPass,
                                     const std::vector<VkImageView>& attachments,
                                     uint32_t width, uint32_t height, uint32_t layers) :
    m_Device(device),
    m_RenderPass(renderPass),
    m_Framebuffer(VK_NULL_HANDLE)
{
    VkFramebufferCreateInfo framebufferCreateInfo{
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .renderPass = renderPass->Get(),
        .attachmentCount = static_cast<uint32_t>(attachments.size()),
        .pAttachments = attachments.empty() ? VK_NULL_HANDLE : attachments.data(),
        .width = width,
        .height = height,
        .layers = layers,
    };

    VkResult res = m_Device->API.vkCreateFramebuffer(device->Get(), &framebufferCreateInfo, nullptr, &m_Framebuffer);
    VK_CHECK_RESULT(res);
}

VulkanFramebuffer::~VulkanFramebuffer()
{
    if (m_Framebuffer != VK_NULL_HANDLE)
    {
        m_Device->API.vkDestroyFramebuffer(m_Device->Get(), m_Framebuffer, nullptr);
    }
}

} // namespace gore