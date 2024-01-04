#pragma once

#include "VulkanIncludes.h"

#include <vector>

namespace gore
{

class VulkanRenderPass;
class VulkanDevice;

class VulkanFramebuffer
{
public:
    VulkanFramebuffer(VulkanDevice* device, VulkanRenderPass* renderPass,
                      const std::vector<VkImageView>& attachments,
                      uint32_t width, uint32_t height, uint32_t layers = 1);
    ~VulkanFramebuffer();

    NON_COPYABLE(VulkanFramebuffer);

    [[nodiscard]] VkFramebuffer Get() const { return m_Framebuffer; }
    [[nodiscard]] VulkanDevice* GetDevice() const { return m_Device; }
    [[nodiscard]] VulkanRenderPass* GetRenderPass() const { return m_RenderPass; }

private:
    VulkanDevice* m_Device;
    VulkanRenderPass* m_RenderPass;
    VkFramebuffer m_Framebuffer;
};

} // namespace gore
