#pragma once

#include "VulkanIncludes.h"

#include <vector>

namespace gore
{

class VulkanDevice;
class VulkanShader;

struct VulkanGraphicsPipelineCreateInfo
{
    VulkanShader* shader;
    // TODO: descriptor set
    // TODO: descriptor pool
    // TODO: pipeline layout

    // fixed function state
    // leave scissors and viewport as dynamic states
    // TODO: replace these to not use Vulkan structs
    VkPipelineVertexInputStateCreateInfo vertexInputInfo;
    VkPipelineInputAssemblyStateCreateInfo inputAssembly;
    VkPipelineRasterizationStateCreateInfo rasterizer;
    VkPipelineDepthStencilStateCreateInfo depthStencil;
    VkPipelineColorBlendStateCreateInfo colorBlending;

    // TODO: refer to VkGraphicsPipelineCreateInfo for more

    uint32_t renderTargetCount;
    std::vector<VkFormat> colorAttachmentFormats;
    VkFormat depthAttachmentFormat;
};

// TODO: this is currently graphics only
class VulkanPipeline
{
public:
    VulkanPipeline(VulkanDevice* device, const VulkanGraphicsPipelineCreateInfo& createInfo);
    ~VulkanPipeline();

    NON_COPYABLE(VulkanPipeline);

    [[nodiscard]] VkPipeline Get() const { return m_Pipeline; }

private:
    VulkanDevice* m_Device;
    VkPipeline m_Pipeline;

    // TODO: move this to another place when we have a working descriptor management system
    // TODO: this is temporary now!
    VkPipelineLayout m_PipelineLayout;
};

} // namespace gore
