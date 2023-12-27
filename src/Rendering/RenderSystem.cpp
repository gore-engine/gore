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
    m_VulkanInstance()
{
    g_RenderSystem = this;
}

RenderSystem::~RenderSystem()
{
    g_RenderSystem = nullptr;
}

void RenderSystem::Initialize()
{
    bool result = m_VulkanInstance.Initialize();


}

void RenderSystem::Update()
{
}

void RenderSystem::Shutdown()
{


    bool result = m_VulkanInstance.Shutdown();
}

void RenderSystem::OnResize(Window* window, int width, int height)
{
}

} // namespace gore
