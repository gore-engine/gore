#include "MeshRenderer.h"

namespace gore::gfx
{
MeshRenderer::MeshRenderer(GameObject* GameObject) noexcept :
    Component(GameObject),
    m_IndexType(IndexType::None),
    m_VertexBuffer(),
    m_IndexBuffer()
{
}
} // namespace gore::gfx