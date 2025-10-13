#pragma once

#include "Math/Matrix4x4.h"
#include "Math/Vector3.h"
#include "Math/Vector4.h"

#include "GeometryPool.h"
#include "MaterialPool.h"

#include <vector>

namespace gore
{
class GameObject;
}

namespace gore::gfx
{
struct InstanceData
{
    Matrix4x4 preModelMatrix = Matrix4x4::Identity; // Pre-model matrix for the instance
    Matrix4x4 modelMatrix    = Matrix4x4::Identity; // Model matrix for the instance

    uint32_t geometryIndex = 0; // Index of the geometry in the GPUScene
    uint32_t materialIndex = 0; // Index of the material in the GPUScene
    uint32_t pad0          = 0; // Padding for alignment
    uint32_t pad1          = 0; // Padding for alignment
};

struct GPUScene
{
public:
    MaterialPool& GetMaterialPool() noexcept { return m_MaterialPool; }

    std::vector<MaterialData> materials;
    std::vector<InstanceData> instances;
    std::vector<GeometryData> geometries;

    void Clear()
    {
        materials.clear();
        instances.clear();
        geometries.clear();

        m_MaterialPool.Reset();
    }

private:
    MaterialPool m_MaterialPool;
};

void ConstructGPUSceneData(
    const std::vector<gore::GameObject*>& gameObjects,
    GPUScene& outScene);

} // namespace gore::gfx