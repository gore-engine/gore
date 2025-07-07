#pragma once

#include "Math/Matrix4x4.h"
#include "Math/Vector3.h"
#include "Math/Vector4.h"

#include <vector>

namespace gore::gfx
{
struct MaterialData
{
    uint32_t albedoTextureIndex;
    uint32_t normalTextureIndex;
    uint32_t maskTextureIndex; // Contains metallic, roughness, and occlusion
    float normalScale; // Scale for normal map

    Vector4 baseColor;
    Vector4 specularColor;
    Vector4 emissiveColor;
    float metallicFactor; // Metallic factor for PBR
    float roughnessFactor; // Roughness factor for PBR
    float occlusionStrength; // Occlusion strength for PBR
    float pad0; // Padding for alignment
}; 

struct GeometryData
{
    uint32_t vertexCount;
    uint32_t indexCount;    
    uint32_t vertexOffset;
    uint32_t indexOffset;   

    uint32_t texcoord0Offset;
    uint32_t texcoord1Offset;
    uint32_t texcoord2Offset;
    uint32_t texcoord3Offset;

    uint32_t normalOffset;
    uint32_t tangentOffset;
    uint32_t pad0;
    uint32_t pad1;
};
    
struct InstanceData
{
    gore::Matrix4x4 preModelMatrix; // Pre-model matrix for the instance
    gore::Matrix4x4 modelMatrix;    // Model matrix for the instance

    uint32_t geometryIndex; // Index of the geometry in the GPUScene
    uint32_t materialIndex; // Index of the material in the GPUScene
    uint32_t pad0;          // Padding for alignment
    uint32_t pad1;          // Padding for alignment
};

struct GPUScene
{
    std::vector<MaterialData> materials;
    std::vector<InstanceData> instances;
    std::vector<GeometryData> geometries;
};

} // namespace gore::gfx