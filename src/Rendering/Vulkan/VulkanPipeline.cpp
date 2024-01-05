#include "Prefix.h"

#include "VulkanPipeline.h"
#include "VulkanRenderPass.h"
#include "VulkanShader.h"
#include "VulkanDevice.h"

namespace gore
{

VulkanPipeline::VulkanPipeline(VulkanDevice* device, const VulkanGraphicsPipelineCreateInfo& createInfo) :
    m_Device(device),
    m_Pipeline(VK_NULL_HANDLE),
    m_PipelineLayout(VK_NULL_HANDLE)
{
    // create temporary renderpass for pipeline creation
    std::vector<VkAttachmentDescription> colorAttachments;
    colorAttachments.reserve(createInfo.renderTargetCount);
    for (uint32_t i = 0; i < createInfo.renderTargetCount; ++i)
    {
        VkAttachmentDescription attachment{
            .flags          = 0,
            .format         = createInfo.colorAttachmentFormats[i],
            .samples        = VK_SAMPLE_COUNT_1_BIT, // TODO: MSAA
            .loadOp         = VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout  = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        };

        colorAttachments.push_back(attachment);
    }

    VkAttachmentDescription depthAttachment{
        .flags          = 0,
        .format         = createInfo.depthAttachmentFormat,
        .samples        = VK_SAMPLE_COUNT_1_BIT, // TODO: MSAA
        .loadOp         = VK_ATTACHMENT_LOAD_OP_LOAD,
        .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_LOAD,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE,
        .initialLayout  = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        .finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };

    VulkanRenderPass renderPass(m_Device, colorAttachments, depthAttachment);

    // TODO: this is temporary now!
    VkPushConstantRange pushConstantRange{
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .offset     = 0,
        .size       = 4 * sizeof(float),
    };

    // create temporary pipeline layout
    // TODO: change this when we have a working descriptor management system
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{
        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext                  = VK_NULL_HANDLE,
        .setLayoutCount         = 0,
        .pSetLayouts            = VK_NULL_HANDLE,
        .pushConstantRangeCount = 1,
        .pPushConstantRanges    = &pushConstantRange,
    };
    VkResult res = m_Device->API.vkCreatePipelineLayout(m_Device->Get(), &pipelineLayoutInfo, VK_NULL_HANDLE, &m_PipelineLayout);
    VK_CHECK_RESULT(res);

    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    shaderStages.reserve(6); // TODO: magic number
    for (uint32_t i = 0; i < 6; ++i)
    {
        auto stage = static_cast<ShaderStage>(1 << i);
        if (HasFlag(createInfo.shader->GetStages(), stage))
        {
            VkPipelineShaderStageCreateInfo shaderStageCreateInfo = createInfo.shader->GetShaderStageCreateInfo(stage);
            shaderStages.push_back(shaderStageCreateInfo);
        }
    }

    VkPipelineViewportStateCreateInfo viewportStateCreateInfo{
        .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext         = VK_NULL_HANDLE,
        .flags         = 0,
        .viewportCount = 1,
        .pViewports    = VK_NULL_HANDLE,
        .scissorCount  = 1,
        .pScissors     = VK_NULL_HANDLE,
    };

    // TODO: MSAA
    VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo{
        .sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .pNext                 = VK_NULL_HANDLE,
        .flags                 = 0,
        .rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable   = VK_FALSE,
        .minSampleShading      = 0.0f,
        .pSampleMask           = VK_NULL_HANDLE,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable      = VK_FALSE,
    };

    VkDynamicState dynamicStates[] =
    {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };

    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{
        .sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .pNext             = VK_NULL_HANDLE,
        .flags             = 0,
        .dynamicStateCount = 2,
        .pDynamicStates    = dynamicStates,
    };

    VkGraphicsPipelineCreateInfo pipelineCreateInfo{
        .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext               = VK_NULL_HANDLE,
        .flags               = 0,
        .stageCount          = static_cast<uint32_t>(shaderStages.size()),
        .pStages             = shaderStages.data(),
        .pVertexInputState   = &createInfo.vertexInputInfo,
        .pInputAssemblyState = &createInfo.inputAssembly,
        .pTessellationState  = VK_NULL_HANDLE,
        .pViewportState      = &viewportStateCreateInfo,
        .pRasterizationState = &createInfo.rasterizer,
        .pMultisampleState   = &multisampleStateCreateInfo,
        .pDepthStencilState  = &createInfo.depthStencil,
        .pColorBlendState    = &createInfo.colorBlending,
        .pDynamicState       = &dynamicStateCreateInfo,
        .layout              = m_PipelineLayout,
        .renderPass          = renderPass.Get(),
        .subpass             = 0,
        .basePipelineHandle  = VK_NULL_HANDLE,
        .basePipelineIndex   = -1,
    };

    res = m_Device->API.vkCreateGraphicsPipelines(m_Device->Get(), VK_NULL_HANDLE, 1, &pipelineCreateInfo, VK_NULL_HANDLE, &m_Pipeline);
    VK_CHECK_RESULT(res);
}

VulkanPipeline::~VulkanPipeline()
{
    if (m_Pipeline != VK_NULL_HANDLE)
    {
        m_Device->API.vkDestroyPipeline(m_Device->Get(), m_Pipeline, VK_NULL_HANDLE);
    }

    // TODO: move this to another place when we have a working descriptor management system
    if (m_PipelineLayout != VK_NULL_HANDLE)
    {
        m_Device->API.vkDestroyPipelineLayout(m_Device->Get(), m_PipelineLayout, VK_NULL_HANDLE);
    }
}

} // namespace gore