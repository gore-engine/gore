#include "Prefix.h"

#define VMA_IMPLEMENTATION
#include "Context.h"

namespace gore::gfx
{

Context::Context(RenderSystem* system, App* app) :
    m_RenderSystem(system)
{
}

Context::~Context()
{
}

void Context::Initialize()
{
}

void Context::Update()
{

}

void Context::Shutdown()
{

}

void Context::OnResize(Window* window, int width, int height)
{

}



} // namespace gore::gfx