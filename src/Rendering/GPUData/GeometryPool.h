#pragma once

#include "Rendering/GPUData/GPUScene.h"
#include "Utilities/Allocator/HashAllocatePool.h"

using namespace gore::utils;

namespace gore::gfx
{
class GeometryPool
{
public:
    GeometryPool() noexcept :
        m_GeometryAllocator(1024)
    {
    }

    ~GeometryPool() = default;

    [[nodiscard]] bool AddGeometry(GeometryData&& geometry, uint32_t& outIndex)
    {
        return m_GeometryAllocator.AddInstance(std::move(geometry), outIndex);
    }

    void RemoveGeometry(const GeometryData& geometry)
    {
        m_GeometryAllocator.RemoveInstance(geometry);
    }

private:
    HashAllocatePool<GeometryData> m_GeometryAllocator;
};
} // namespace gore::gfx