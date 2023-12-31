#include "Prefix.h"

#include "RenderSystem.h"

#include "Core/Log.h"
#include "Core/App.h"
#include "Windowing/Window.h"
#include "Rendering/Vulkan/VulkanInstance.h"
#include "Rendering/Vulkan/VulkanDevice.h"
#include "Rendering/Vulkan/VulkanCommandPool.h"
#include "Rendering/Vulkan/VulkanCommandBuffer.h"

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
    m_VulkanSwapchain(nullptr)
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

    m_VulkanSurface = new VulkanSurface(m_VulkanDevice, m_App->GetWindow());
    m_VulkanSwapchain = new VulkanSwapchain(m_VulkanSurface, 3);
}

void RenderSystem::Update()
{
    uint32_t bufferIndex = m_VulkanSwapchain->GetCurrentBufferIndex();
    VulkanImage* image   = m_VulkanSwapchain->GetBuffer(bufferIndex);

    VulkanQueue graphicsQueue = m_VulkanDevice->GetQueue(VulkanQueueType::Graphics);

    VulkanCommandPool* commandPool = VulkanCommandPool::GetOrCreate(m_VulkanDevice, graphicsQueue.GetFamilyIndex());

    VulkanCommandBuffer commandBuffer(commandPool, true);
    commandBuffer.Begin();

    commandBuffer.End();

    m_VulkanSwapchain->Present({});
}

void RenderSystem::Shutdown()
{
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
