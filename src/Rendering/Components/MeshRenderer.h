#pragma once

#include "Prefix.h"
#include "Export.h"

#include "Object/Component.h"

#include "Rendering/Buffer.h"
#include "Rendering/DynamicBuffer.h"
#include "Rendering/BindGroup.h"
#include "Rendering/Components/Material.h"
#include "Rendering/Utils/GeometryUtils.h"

#include "Rendering/System/MeshRendererSystem.h"

namespace gore::renderer
{
using namespace gfx;

ENGINE_CLASS(MeshRenderer) final :
    public Component
{
public:
    NON_COPYABLE(MeshRenderer)

    explicit MeshRenderer(GameObject * GameObject) noexcept;
    ~MeshRenderer() override;

    void LoadMesh(const std::string& name, uint32_t meshIndex = 0, ShaderChannel channel = ShaderChannel::Default);

    void UploadMeshData();

    void SetVertexData(const void* data, size_t size);
    void SetIndexData(const void* data, size_t size);

    bool IsValid() const;

    [[nodiscard]] const void* GetVertexData() const;
    [[nodiscard]] const void* GetIndexData() const;

    [[nodiscard]] size_t GetVertexDataSize() const;
    [[nodiscard]] size_t GetIndexDataSize() const;

    [[nodiscard]] bool HasVertexData() const;
    [[nodiscard]] bool HasIndexData() const;

    GETTER_REF(Material, Material)
    GETTER_SETTER(IndexType, IndexType)
    GETTER_SETTER(DynamicBufferHandle, DynamicBuffer)
    GETTER_SETTER(uint32_t, DynamicBufferOffset)

    GETTER_SETTER(BufferHandle, VertexBuffer)
    GETTER_SETTER(uint32_t, VertexCount)
    GETTER_SETTER(uint32_t, VertexOffset)

    GETTER_SETTER(BufferHandle, IndexBuffer)
    GETTER_SETTER(uint32_t, IndexCount)
    GETTER_SETTER(uint32_t, IndexOffset)
    
    GETTER_SETTER(BindGroupHandle, BindGroup)

    void Start() override;
    void Update() override;

private:
    void DeleteCPUMeshData();
    void DeleteGPUData();

    RendererHandle m_RendererHandle;

    Material m_Material;

    DynamicBufferHandle m_DynamicBuffer;
    uint32_t m_DynamicBufferOffset;

    // Mesh data
    IndexType m_IndexType;

    BufferHandle m_VertexBuffer;
    uint32_t m_VertexCount;
    uint32_t m_VertexOffset;

    BufferHandle m_IndexBuffer;
    uint32_t m_IndexCount;
    uint32_t m_IndexOffset;

    // Material data
    BindGroupHandle m_BindGroup;
};
} // namespace gore::renderer