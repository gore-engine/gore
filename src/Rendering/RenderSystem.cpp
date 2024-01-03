#include "Prefix.h"

#include "RenderSystem.h"

#include "Core/Log.h"
#include "Core/App.h"
#include "Core/Time.h"
#include "Windowing/Window.h"
#include "Rendering/Vulkan/VulkanInstance.h"
#include "Rendering/Vulkan/VulkanDevice.h"
#include "Rendering/Vulkan/VulkanCommandPool.h"
#include "Rendering/Vulkan/VulkanCommandBuffer.h"
#include "Rendering/Vulkan/VulkanImage.h"
#include "Rendering/Vulkan/VulkanSynchronization.h"
#include "Rendering/Vulkan/VulkanRenderPass.h"
#include "Rendering/Vulkan/VulkanFramebuffer.h"

#include <cmath>
#include <iostream>
#include <vector>
#include <algorithm>

namespace gore
{

static RenderSystem* g_RenderSystem = nullptr;

RenderSystem::RenderSystem(gore::App* app) :
    System(app),
    m_VulkanInstance(nullptr),
    m_VulkanDevice(nullptr),
    m_VulkanSurface(nullptr),
    m_VulkanSwapchain(nullptr),
    m_RenderFinishedSemaphores(),
    m_TriangleShader(nullptr),
    m_TrianglePipeline(nullptr)
{
    g_RenderSystem = this;
}

RenderSystem::~RenderSystem()
{
    g_RenderSystem = nullptr;
}

void RenderSystem::Initialize()
{
    m_VulkanInstance = new VulkanInstance(m_App);

    std::vector<VulkanPhysicalDevice> physicalDevices = m_VulkanInstance->GetPhysicalDevices();
    std::sort(physicalDevices.begin(), physicalDevices.end(), [](const VulkanPhysicalDevice& a, const VulkanPhysicalDevice& b)
              { return a.Score() > b.Score(); });

    m_VulkanDevice = new VulkanDevice(m_VulkanInstance, physicalDevices[0]);

    m_VulkanSurface   = new VulkanSurface(m_VulkanDevice, m_App->GetWindow());
    m_VulkanSwapchain = new VulkanSwapchain(m_VulkanSurface, 3);

    m_RenderFinishedSemaphores.resize(m_VulkanSwapchain->GetImageCount());
    for (uint32_t i = 0; i < m_VulkanSwapchain->GetImageCount(); ++i)
    {
        m_RenderFinishedSemaphores[i] = new VulkanSemaphore(m_VulkanDevice);
    }

    m_TriangleShader = new VulkanShader(m_VulkanDevice, "sample/triangle", ShaderStage::Vertex | ShaderStage::Fragment);
    m_TriangleShader->SetEntryPoint(ShaderStage::Vertex, "vs");
    m_TriangleShader->SetEntryPoint(ShaderStage::Fragment, "ps");

    VulkanGraphicsPipelineCreateInfo pipelineCreateInfo{
        .shader = m_TriangleShader,
 // TODO: descriptor set
  // TODO: descriptor pool
  // TODO: pipeline layout
        .vertexInputInfo = {
                            .sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
                            .pNext                           = VK_NULL_HANDLE,
                            .flags                           = 0,
                            .vertexBindingDescriptionCount   = 0,
                            .pVertexBindingDescriptions      = VK_NULL_HANDLE,
                            .vertexAttributeDescriptionCount = 0,
                            .pVertexAttributeDescriptions    = VK_NULL_HANDLE,
                            },
        .inputAssembly = {
                            .sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
                            .pNext                  = VK_NULL_HANDLE,
                            .flags                  = 0,
                            .topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                            .primitiveRestartEnable = VK_FALSE,
                            },
        .rasterizer = {
                            .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
                            .pNext                   = VK_NULL_HANDLE,
                            .flags                   = 0,
                            .depthClampEnable        = VK_FALSE,
                            .rasterizerDiscardEnable = VK_FALSE,
                            .polygonMode             = VK_POLYGON_MODE_FILL,
                            .cullMode                = VK_CULL_MODE_BACK_BIT,
                            .frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE,
                            .depthBiasEnable         = VK_FALSE,
                            .depthBiasConstantFactor = 0.0f,
                            .depthBiasClamp          = 0.0f,
                            .depthBiasSlopeFactor    = 0.0f,
                            .lineWidth               = 1.0f,
                            },
        .depthStencil = {
                            .sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
                            .pNext                 = VK_NULL_HANDLE,
                            .flags                 = 0,
                            .depthTestEnable       = VK_TRUE,
                            .depthWriteEnable      = VK_TRUE,
                            .depthCompareOp        = VK_COMPARE_OP_LESS,
                            .depthBoundsTestEnable = VK_FALSE,
                            .stencilTestEnable     = VK_FALSE,
                            .front                 = {},
                            .back                  = {},
                            .minDepthBounds        = 0.0f,
                            .maxDepthBounds        = 1.0f,
                            },
        .colorBlending = {
                            .sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
                            .pNext           = VK_NULL_HANDLE,
                            .flags           = 0,
                            .logicOpEnable   = VK_FALSE,
                            .logicOp         = VK_LOGIC_OP_COPY,
                            .attachmentCount = 0,
                            .pAttachments    = VK_NULL_HANDLE,
                            .blendConstants  = {0.0f, 0.0f, 0.0f, 0.0f},
                            },
        .renderTargetCount      = 1,
        .colorAttachmentFormats = {m_VulkanSwapchain->GetImageFormat()},
        .depthAttachmentFormat  = VK_FORMAT_UNDEFINED,
    };

    m_TrianglePipeline = new VulkanPipeline(m_VulkanDevice, pipelineCreateInfo);
}

void RenderSystem::Update()
{
    uint32_t bufferIndex        = m_VulkanSwapchain->GetCurrentBufferIndex();
    VulkanImage* swapchainImage = m_VulkanSwapchain->GetBuffer(bufferIndex);

    VulkanQueue graphicsQueue = m_VulkanDevice->GetQueue(VulkanQueueType::Graphics);

    VkAttachmentDescription colorAttachmentDescription{
        .flags          = 0,
        .format         = swapchainImage->GetFormat(),
        .samples        = VK_SAMPLE_COUNT_1_BIT, // TODO: MSAA
        .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout  = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };
    VulkanRenderPass renderPass(m_VulkanDevice, {colorAttachmentDescription}, {});
    VulkanFramebuffer framebuffer(m_VulkanDevice, &renderPass, {swapchainImage->GetRTV()}, m_VulkanSwapchain->GetWidth(), m_VulkanSwapchain->GetHeight());

    VulkanCommandPool* commandPool = VulkanCommandPool::GetOrCreate(m_VulkanDevice, graphicsQueue.GetFamilyIndex());

    VulkanCommandBuffer commandBuffer(commandPool, true);
    commandBuffer.Begin();

    VulkanResourceBarrier transitionToRTBarrier{
        .type         = ResourceBarrierType::Image,
        .image        = swapchainImage,
        .currentState = ResourceState::Present,
        .newState     = ResourceState::RenderTarget,
    };

    commandBuffer.Barrier({transitionToRTBarrier}, VulkanQueueType::Graphics);

    // TODO: Hide these pure vulkan calls
    float totalTime              = GetTotalTime();
    float val1                   = sinf(totalTime) * 0.5f + 0.5f;
    float val2                   = sinf(totalTime + 2 * 3.14159f / 3) * 0.5f + 0.5f;
    float val3                   = sinf(totalTime + 4 * 3.14159f / 3) * 0.5f + 0.5f;
    VkClearColorValue clearColor = {val1, val2, val3, 1.0f};
    VkClearValue clearValue      = {.color = clearColor};

    VkExtent2D swapchainExtent{
        .width  = m_VulkanSwapchain->GetWidth(),
        .height = m_VulkanSwapchain->GetHeight(),
    };

    VkRenderPassBeginInfo renderPassBeginInfo{
        .sType       = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .pNext       = nullptr,
        .renderPass  = renderPass.Get(),
        .framebuffer = framebuffer.Get(),
        .renderArea  = {
                        .offset = {0, 0},
                        .extent = swapchainExtent,
                        },
        .clearValueCount = 1,
        .pClearValues    = &clearValue,
    };

    m_VulkanDevice->API.vkCmdBeginRenderPass(commandBuffer.Get(), &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    m_VulkanDevice->API.vkCmdBindPipeline(commandBuffer.Get(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_TrianglePipeline->Get());

    VkViewport viewport{
        .x        = 0.0f,
        .y        = 0.0f,
        .width    = static_cast<float>(swapchainExtent.width),
        .height   = static_cast<float>(swapchainExtent.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };
    m_VulkanDevice->API.vkCmdSetViewport(commandBuffer.Get(), 0, 1, &viewport);

    VkRect2D scissor{
        .offset = {0, 0},
        .extent = swapchainExtent,
    };
    m_VulkanDevice->API.vkCmdSetScissor(commandBuffer.Get(), 0, 1, &scissor);

    m_VulkanDevice->API.vkCmdDraw(commandBuffer.Get(), 3, 1, 0, 0);

    m_VulkanDevice->API.vkCmdEndRenderPass(commandBuffer.Get());
    //

    VulkanResourceBarrier transitionToPresentBarrier{
        .type         = ResourceBarrierType::Image,
        .image        = swapchainImage,
        .currentState = ResourceState::RenderTarget,
        .newState     = ResourceState::Present,
    };

    commandBuffer.Barrier({transitionToPresentBarrier}, VulkanQueueType::Graphics);

    commandBuffer.End();

    graphicsQueue.Submit({&commandBuffer}, {}, {m_RenderFinishedSemaphores[bufferIndex]});

    m_VulkanSwapchain->Present({m_RenderFinishedSemaphores[bufferIndex]});
}

void RenderSystem::Shutdown()
{
    delete m_TrianglePipeline;
    delete m_TriangleShader;

    for (uint32_t i = 0; i < m_RenderFinishedSemaphores.size(); ++i)
    {
        delete m_RenderFinishedSemaphores[i];
    }

    VulkanCommandPool::ClearAll();

    delete m_VulkanSwapchain;
    delete m_VulkanSurface;

    delete m_VulkanDevice;

    delete m_VulkanInstance;
}

void RenderSystem::OnResize(Window* window, int width, int height)
{
}

} // namespace gore
