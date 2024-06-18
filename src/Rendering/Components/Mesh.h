#pragma once

#include "Prefix.h"
#include "Export.h"

#include "Object/Object.h"
#include "Rendering/Buffer.h"
#include "Math/Vector2.h"
#include "Math/Vector3.h"
#include "Math/Vector4.h"

namespace gore::gfx
{
enum class MeshChannel : uint8_t
{
    Position  = 0,
    Normal    = 1,
    Tangent   = 1 << 1,
    Color     = 1 << 2,
    TexCoord0 = 1 << 3,
    TexCoord1 = 1 << 4,
    TexCoord2 = 1 << 5,
    TexCoord3 = 1 << 6,

    Default = Position | Normal | TexCoord0,
};

FLAG_ENUM_CLASS(MeshChannel, uint8_t)

class MeshConstants
{
public:
    static const Vector3 DefaultPosition;
    static const Vector3 DefaultNormal;
    static const Vector4 DefaultTangent;
    static const Vector4 DefaultColor;
    static const Vector2 DefaultTexCoord;
};

const Vector3 MeshConstants::DefaultPosition = Vector3(0.0f, 0.0f, 0.0f);
const Vector3 MeshConstants::DefaultNormal = Vector3(0.0f, 1.0f, 0.0f);
const Vector4 MeshConstants::DefaultTangent = Vector4(0.0f, 0.0f, 1.0f, 1.0f);
const Vector4 MeshConstants::DefaultColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
const Vector2 MeshConstants::DefaultTexCoord = Vector2(0.0f, 0.0f); 

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