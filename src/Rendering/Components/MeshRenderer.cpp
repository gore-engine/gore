#include "MeshRenderer.h"

namespace gore::renderer
{
MeshRenderer::MeshRenderer(GameObject* GameObject) noexcept :
    Component(GameObject),
    m_IndexType(IndexType::None),
    m_VertexBuffer(),
    m_VertexCount(0),
    m_IndexBuffer(),
    m_IndexCount(0),
    m_RendererHandle(RendererHandle::Invalid())
{
    m_RendererHandle = MeshRendererSystem::GetInstance()->GetRendererHandle();
}

MeshRenderer::~MeshRenderer()
{
    MeshRendererSystem::GetInstance()->FreeRendererHandle(m_RendererHandle);
}

bool MeshRenderer::IsValid() const
{
    return m_VertexBuffer.empty() == false 
        && m_IndexBuffer.empty() == false
        && m_VertexCount > 0
        && m_IndexCount > 0;
}

void MeshRenderer::Start()
{
}

void MeshRenderer::Update()
{
}
} // namespace gore::renderer