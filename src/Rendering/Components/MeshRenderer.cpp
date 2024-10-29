#include "MeshRenderer.h"

#include "Rendering/RenderContext.h"

namespace gore::renderer
{
MeshRenderer::MeshRenderer(GameObject* GameObject) noexcept :
    Component(GameObject),
    m_IndexType(IndexType::None),
    m_VertexBuffer(),
    m_VertexCount(0),
    m_VertexOffset(0),
    m_IndexBuffer(),
    m_IndexCount(0),
    m_IndexOffset(0),
    m_DynamicBuffer(),
    m_DynamicBufferOffset(0)
{
    // m_RendererHandle = MeshRendererSystem::GetInstance()->GetRendererHandle();
}

MeshRenderer::~MeshRenderer()
{
    // MeshRendererSystem::GetInstance()->FreeRendererHandle(m_RendererHandle);
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

void MeshRenderer::LoadMesh(const std::string& name, uint32_t meshIndex, ShaderChannel channel)
{
    auto& renderContext = *RenderContext::GetInstance();

    renderContext.LoadMeshToMeshRenderer(name, *this, meshIndex, channel);
}
} // namespace gore::renderer