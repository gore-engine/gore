#include "MeshRenderer.h"

namespace gore::gfx
{
MeshRenderer::MeshRenderer(GameObject* GameObject) noexcept :
    Component(GameObject),
    m_IndexType(IndexType::None),
    m_VertexBuffer(),
    m_VertexCount(0),
    m_IndexBuffer(),
    m_IndexCount(0)
{
}

void MeshRenderer::Start()
{
}

void MeshRenderer::Update()
{
}
} // namespace gore::gfx