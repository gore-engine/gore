#include "RendererManager.h"

namespace gore::renderer
{
SINGLETON_IMPL(RendererManager)

RendererManager::RendererManager() :
    m_MeshRendererAllocator(std::make_unique<utils::ArrayAllocator>())
{
}

RendererHandle RendererManager::GetRendererHandle()
{
    return RendererHandle(m_MeshRendererAllocator->Allocate());
}

void RendererManager::FreeRendererHandle(RendererHandle handle)
{
    m_MeshRendererAllocator->Free(handle.GetIndex());
}
} // namespace gore::renderer