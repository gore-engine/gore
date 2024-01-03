#pragma once

#include "VulkanIncludes.h"

#include <vector>

namespace gore
{

class VulkanDevice;

class VulkanRenderPass
{
public:
    // TODO: use non-Vulkan structs in the future
    VulkanRenderPass(VulkanDevice* device,
                     const std::vector<VkAttachmentDescription>& colorAttachments,
                     VkAttachmentDescription depthAttachment);
    ~VulkanRenderPass();

    NON_COPYABLE(VulkanRenderPass);

    [[nodiscard]] VkRenderPass Get() const { return m_RenderPass; }
    [[nodiscard]] VulkanDevice* GetDevice() const { return m_Device; }

private:
    VulkanDevice* m_Device;
    VkRenderPass m_RenderPass;
};

} // namespace gore
