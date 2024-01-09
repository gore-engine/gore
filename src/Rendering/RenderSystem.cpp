#include "Prefix.h"

#include "RenderSystem.h"

#include "Graphics/Context.h"

namespace gore
{

static RenderSystem* g_RenderSystem = nullptr;

RenderSystem::RenderSystem(gore::App* app) :
    System(app),
    m_Context(nullptr)
{
    g_RenderSystem = this;
}

RenderSystem::~RenderSystem()
{
    g_RenderSystem = nullptr;
}

void RenderSystem::Initialize()
{
    m_Context = new graphics::Context(this, m_App);
    m_Context->Initialize();
}

void RenderSystem::Update()
{
    m_Context->Update();
}

void RenderSystem::Shutdown()
{
    m_Context->Shutdown();
}

void RenderSystem::OnResize(Window* window, int width, int height)
{
    m_Context->OnResize(window, width, height);
}

} // namespace gore
