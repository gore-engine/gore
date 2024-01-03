#include "Prefix.h"

#include "VulkanRenderPass.h"
#include "VulkanDevice.h"

namespace gore
{

VulkanRenderPass::VulkanRenderPass(VulkanDevice* device,
                                   const std::vector<VkAttachmentDescription>& colorAttachments,
                                   VkAttachmentDescription depthAttachment) :
    m_Device(device),
    m_RenderPass(VK_NULL_HANDLE)
{
    std::vector<VkAttachmentDescription> attachments;
    attachments.reserve(colorAttachments.size() + 1);

    std::vector<VkAttachmentReference> colorAttachmentRefs;
    colorAttachmentRefs.reserve(colorAttachments.size());

    VkAttachmentReference depthAttachmentRef{};

    for (uint32_t i = 0; i < colorAttachments.size(); ++i)
    {
        auto& attachment = colorAttachments[i];
        attachments.push_back(attachment);

        VkAttachmentReference attachmentRef{};
        attachmentRef.attachment = i;
        attachmentRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachmentRefs.push_back(attachmentRef);
    }

    if (depthAttachment.format != VK_FORMAT_UNDEFINED)
    {
        attachments.push_back(depthAttachment);

        depthAttachmentRef.attachment = static_cast<uint32_t>(attachments.size() - 1);
        depthAttachmentRef.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }

    VkSubpassDescription subpassDesc{
        .flags                   = 0,
        .pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .inputAttachmentCount    = 0,
        .pInputAttachments       = VK_NULL_HANDLE,
        .colorAttachmentCount    = static_cast<uint32_t>(colorAttachmentRefs.size()),
        .pColorAttachments       = colorAttachmentRefs.empty() ? VK_NULL_HANDLE : colorAttachmentRefs.data(),
        .pResolveAttachments     = VK_NULL_HANDLE,
        .pDepthStencilAttachment = depthAttachment.format == VK_FORMAT_UNDEFINED ? VK_NULL_HANDLE : &depthAttachmentRef,
        .preserveAttachmentCount = 0,
        .pPreserveAttachments    = VK_NULL_HANDLE,
    };

    VkRenderPassCreateInfo renderPassCreateInfo{
        .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext           = VK_NULL_HANDLE,
        .flags           = 0,
        .attachmentCount = static_cast<uint32_t>(attachments.size()),
        .pAttachments    = attachments.data(),
        .subpassCount    = 1,
        .pSubpasses      = &subpassDesc,
        .dependencyCount = 0,
        .pDependencies   = VK_NULL_HANDLE,
    };

    VkResult res = m_Device->API.vkCreateRenderPass(m_Device->Get(), &renderPassCreateInfo, VK_NULL_HANDLE, &m_RenderPass);
    VK_CHECK_RESULT(res);
}

VulkanRenderPass::~VulkanRenderPass()
{
    if (m_RenderPass != VK_NULL_HANDLE)
    {
        m_Device->API.vkDestroyRenderPass(m_Device->Get(), m_RenderPass, VK_NULL_HANDLE);
    }
}

} // namespace gore