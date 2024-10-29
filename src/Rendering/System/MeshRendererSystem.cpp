#include "MeshRendererSystem.h"

namespace gore::renderer
{
MeshRendererSystem::MeshRendererSystem() :
    m_MeshRendererAllocator(std::make_unique<utils::ArrayAllocator>())
{
}

RendererHandle MeshRendererSystem::GetRendererHandle()
{
    return RendererHandle(m_MeshRendererAllocator->Allocate());
}

void MeshRendererSystem::FreeRendererHandle(RendererHandle handle)
{
    m_MeshRendererAllocator->Free(handle.GetIndex());
}
} // namespace gore::renderer