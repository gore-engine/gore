#pragma once

#include "Prefix.h"
#include "Export.h"

#include "Object/Object.h"

namespace gore::gfx
{
ENGINE_CLASS(Mesh) :
    public Object
{
public:
    NON_COPYABLE(Mesh);

    Mesh();
    ~Mesh() override;

    void UploadMeshData();

    void SetVertexData(const void* data, size_t size);
    void SetIndexData(const void* data, size_t size);

    [[nodiscard]] const void* GetVertexData() const;
    [[nodiscard]] const void* GetIndexData() const;

    [[nodiscard]] size_t GetVertexDataSize() const;
    [[nodiscard]] size_t GetIndexDataSize() const;

    [[nodiscard]] bool HasVertexData() const;
    [[nodiscard]] bool HasIndexData() const;

private:
    void DeleteCPUMeshData();

    // CPU vertex data
    // CPU index data
};
} // namespace gore::gfx