#pragma once

#include "Prefix.h"
#include "Export.h"

#include <memory>

#include "Rendering/Handle.h"
#include "Utilities/Allocator/ArrayAllocator.h"

namespace gore::renderer
{
ENGINE_STRUCT(RendererHandle)
{
public:
    explicit RendererHandle(uint32_t index) :
        index(index)
    {
    }

    [[nodiscard]] uint32_t GetIndex() const
    {
        return index;
    }

    bool operator==(const RendererHandle& other) const
    {
        return index == other.index;
    }

    bool operator!=(const RendererHandle& other) const
    {
        return index != other.index;
    }

    static RendererHandle Invalid()
    {
        return RendererHandle(UINT32_MAX);
    }

    [[nodiscard]] bool IsValid() const
    {
        return index != UINT32_MAX;
    }
    
private:
    uint32_t index;
};

ENGINE_CLASS(MeshRendererSystem)
{
public:
    MeshRendererSystem();

    RendererHandle GetRendererHandle();
    void FreeRendererHandle(RendererHandle handle);

private:
    std::unique_ptr<utils::ArrayAllocator> m_MeshRendererAllocator;
};
} // namespace gore::renderer