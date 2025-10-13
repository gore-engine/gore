#pragma once

#include "Math/Vector4.h"

#include "Utilities/Allocator/HashAllocatePool.h"

using namespace gore::utils;

namespace gore::gfx
{
struct MaterialData
{
    uint32_t albedoTextureIndex = 0xFFFFFFFF; // Index of the albedo texture in the GPUScene
    uint32_t normalTextureIndex = 0xFFFFFFFF;
    uint32_t maskTextureIndex   = 0xFFFFFFFF; // Contains metallic, roughness, and occlusion
    float normalScale           = 1.0f;       // Scale for normal map

    Vector4 baseColor       = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    Vector4 specularColor   = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    Vector4 emissiveColor   = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
    float metallicFactor    = 0.0f; // Metallic factor for PBR
    float roughnessFactor   = 1.0f; // Roughness factor for PBR
    float occlusionStrength = 1.0f; // Occlusion strength for PBR
    float pad0              = 0.0f; // Padding for alignment
};

inline bool operator==(const MaterialData& lhs, const MaterialData& rhs) noexcept
{
    return lhs.albedoTextureIndex == rhs.albedoTextureIndex &&
           lhs.normalTextureIndex == rhs.normalTextureIndex &&
           lhs.maskTextureIndex == rhs.maskTextureIndex &&
           lhs.normalScale == rhs.normalScale &&
           lhs.baseColor == rhs.baseColor &&
           lhs.specularColor == rhs.specularColor &&
           lhs.emissiveColor == rhs.emissiveColor &&
           lhs.metallicFactor == rhs.metallicFactor &&
           lhs.roughnessFactor == rhs.roughnessFactor &&
           lhs.occlusionStrength == rhs.occlusionStrength;
}
} // namespace gore::gfx

namespace std
{
template<>
struct hash<gore::gfx::MaterialData>
{
    size_t operator()(const gore::gfx::MaterialData& material) const noexcept
    {
        size_t h1 = std::hash<uint32_t>{}(material.albedoTextureIndex);
        size_t h2 = std::hash<uint32_t>{}(material.normalTextureIndex);
        size_t h3 = std::hash<uint32_t>{}(material.maskTextureIndex);
        size_t h4 = std::hash<float>{}(material.normalScale);
        size_t h5 = std::hash<gore::Vector4>{}(material.baseColor);
        size_t h6 = std::hash<gore::Vector4>{}(material.specularColor);
        size_t h7 = std::hash<gore::Vector4>{}(material.emissiveColor);
        size_t h8 = std::hash<float>{}(material.metallicFactor);
        size_t h9 = std::hash<float>{}(material.roughnessFactor);
        size_t h10 = std::hash<float>{}(material.occlusionStrength);
        
        // Combine hashes using a simple method
        return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3) ^ (h5 << 4) ^ 
               (h6 << 5) ^ (h7 << 6) ^ (h8 << 7) ^ (h9 << 8) ^ (h10 << 9);
    }
};
} // namespace std

namespace gore::gfx
{
    using MaterialPool = HashAllocatePool<MaterialData>;
} // namespace gore::gfx