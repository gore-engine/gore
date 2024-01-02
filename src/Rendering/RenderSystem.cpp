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
    m_RenderFinishedSemaphores()
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
}

void RenderSystem::Update()
{
    uint32_t bufferIndex        = m_VulkanSwapchain->GetCurrentBufferIndex();
    VulkanImage* swapchainImage = m_VulkanSwapchain->GetBuffer(bufferIndex);

    VulkanQueue graphicsQueue = m_VulkanDevice->GetQueue(VulkanQueueType::Graphics);

    VulkanCommandPool* commandPool = VulkanCommandPool::GetOrCreate(m_VulkanDevice, graphicsQueue.GetFamilyIndex());

    VulkanCommandBuffer commandBuffer(commandPool, true);
    commandBuffer.Begin();

    VulkanResourceBarrier transitionToRTBarrier{
        .type         = ResourceBarrierType::Image,
        .image        = swapchainImage,
        .currentState = ResourceState::Present,
        .newState     = ResourceState::TransferDest,
    };

    commandBuffer.Barrier({transitionToRTBarrier}, VulkanQueueType::Graphics);

    // TODO: Hide these pure vulkan calls
    float totalTime = GetTotalTime();
    float val1 = sinf(totalTime) * 0.5f + 0.5f;
    float val2 = sinf(totalTime + 2 * 3.14159f / 3) * 0.5f + 0.5f;
    float val3 = sinf(totalTime + 4 * 3.14159f / 3) * 0.5f + 0.5f;
    VkClearColorValue clearColor = {val1, val2, val3, 1.0f};
    VkImageSubresourceRange subresourceRange{
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1
    };
    m_VulkanDevice->API.vkCmdClearColorImage(commandBuffer.Get(), swapchainImage->Get(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColor, 1, &subresourceRange);
    //

    VulkanResourceBarrier transitionToPresentBarrier{
        .type         = ResourceBarrierType::Image,
        .image        = swapchainImage,
        .currentState = ResourceState::TransferDest,
        .newState     = ResourceState::Present,
    };

    commandBuffer.Barrier({transitionToPresentBarrier}, VulkanQueueType::Graphics);

    commandBuffer.End();

    graphicsQueue.Submit({&commandBuffer}, {}, {m_RenderFinishedSemaphores[bufferIndex]});

    m_VulkanSwapchain->Present({m_RenderFinishedSemaphores[bufferIndex]});
}

void RenderSystem::Shutdown()
{
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
