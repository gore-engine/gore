#pragma once

#include "Prefix.h"
#include "Export.h"

#include "Object/Component.h"

#include "Rendering/Buffer.h"
#include "Rendering/BindGroup.h"
#include "Rendering/Utils/GeometryUtils.h"

namespace gore::renderer
{
using namespace gfx;

ENGINE_CLASS(MeshRenderer) final:
    public Component
{
public:
    NON_COPYABLE(MeshRenderer)

    explicit MeshRenderer(GameObject* GameObject) noexcept;
    ~MeshRenderer() override = default;

    void UploadMeshData();

    void SetVertexData(const void* data, size_t size);
    void SetIndexData(const void* data, size_t size);

    [[nodiscard]] const void* GetVertexData() const;
    [[nodiscard]] const void* GetIndexData() const;

    [[nodiscard]] size_t GetVertexDataSize() const;
    [[nodiscard]] size_t GetIndexDataSize() const;

    [[nodiscard]] bool HasVertexData() const;
    [[nodiscard]] bool HasIndexData() const;

    GETTER_SETTER(IndexType, IndexType)
    GETTER_SETTER(BufferHandle, VertexBuffer)
    GETTER_SETTER(BufferHandle, IndexBuffer)
    GETTER_SETTER(uint32_t, VertexCount)
    GETTER_SETTER(uint32_t, IndexCount)
    GETTER_SETTER(BindGroupHandle, BindGroup)

    void Start() override;
    void Update() override;

private:
    void DeleteCPUMeshData();
    void DeleteGPUData();

    // Mesh data
    IndexType m_IndexType;
    
    BufferHandle m_VertexBuffer;
    uint32_t m_VertexCount;

    BufferHandle m_IndexBuffer;
    uint32_t m_IndexCount;

    // Material data
    BindGroupHandle m_BindGroup;
};
} // namespace gore::renderer