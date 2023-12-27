#include "Prefix.h"

#include "RenderSystem.h"

#include "Core/Log.h"
#include "Rendering/Vulkan/VulkanInstance.h"

#include <iostream>
#include <vector>

namespace gore
{

static RenderSystem* g_RenderSystem = nullptr;

RenderSystem::RenderSystem(gore::App* app) :
    System(app),
    m_Instance(VK_NULL_HANDLE),
    m_PhysicalDevice(VK_NULL_HANDLE),
    m_Device(VK_NULL_HANDLE)
{
    g_RenderSystem = this;
}

RenderSystem::~RenderSystem()
{
    g_RenderSystem = nullptr;
}

void RenderSystem::Initialize()
{
    bool result = InitializeVulkanInstance(&m_Instance);


}

void RenderSystem::Update()
{
}

void RenderSystem::Shutdown()
{


    bool result = ShutdownVulkanInstance(m_Instance);
}

void RenderSystem::OnResize(Window* window, int width, int height)
{
}

} // namespace gore
