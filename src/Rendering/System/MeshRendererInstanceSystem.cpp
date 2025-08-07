#include "MeshRendererInstanceSystem.h"

namespace gore::renderer
{
SINGLETON_IMPL(MeshRendererInstanceSystem)

MeshRendererInstanceSystem::MeshRendererInstanceSystem() :
    m_MeshRendererAllocator(std::make_unique<utils::ArrayAllocator>())
{
    g_Instance = this;
}

InstanceHandle MeshRendererInstanceSystem::GetRendererHandle()
{
    return InstanceHandle(m_MeshRendererAllocator->Allocate());
}

void MeshRendererInstanceSystem::FreeRendererHandle(InstanceHandle handle)
{
    m_MeshRendererAllocator->Free(handle.GetIndex());
}
} // namespace gore::renderer