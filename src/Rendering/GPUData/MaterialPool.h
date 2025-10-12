#pragma once

#include "Rendering/GPUData/GPUScene.h"
#include "Utilities/Allocator/HashAllocatePool.h"

using namespace gore::utils;

namespace gore::gfx
{
class MaterialPool
{
public:
    MaterialPool() noexcept :
        m_MaterialAllocator(1024)
    {
    }

    ~MaterialPool() = default;

    [[nodiscard]] bool AddMaterial(MaterialData&& material, uint32_t& outIndex)
    {
        return m_MaterialAllocator.AddInstance(std::move(material), outIndex);
    }

    void RemoveMaterial(const MaterialData& material)
    {
        m_MaterialAllocator.RemoveInstance(material);
    }

private:
    HashAllocatePool<MaterialData> m_MaterialAllocator;
};
} // namespace gore::gfx