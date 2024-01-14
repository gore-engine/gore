#include "RenderContext.h"

gore::RenderContext::RenderContext(vk::raii::Device* device) :
    m_device(device)
{
}

gore::RenderContext::~RenderContext()
{
    clear();
}

void gore::RenderContext::clear()
{
}
