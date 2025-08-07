#pragma once

#include "Prefix.h"

#include <memory>

#include "Rendering/Handle.h"
#include "Utilities/Allocator/ArrayAllocator.h"

namespace gore::renderer
{
struct InstanceHandle
{
public:
    InstanceHandle() :
        index(UINT32_MAX)
    {
    }

    explicit InstanceHandle(uint32_t index) :
        index(index)
    {
    }

    [[nodiscard]] uint32_t GetIndex() const
    {
        return index;
    }

    bool operator==(const InstanceHandle& other) const
    {
        return index == other.index;
    }

    bool operator!=(const InstanceHandle& other) const
    {
        return index != other.index;
    }

    static InstanceHandle Invalid()
    {
        return InstanceHandle(UINT32_MAX);
    }

    [[nodiscard]] bool IsValid() const
    {
        return index != UINT32_MAX;
    }
    
private:
    uint32_t index;
};

class MeshRendererInstanceSystem
{
    SINGLETON(MeshRendererInstanceSystem)

public:
    MeshRendererInstanceSystem();

    InstanceHandle GetRendererHandle();
    void FreeRendererHandle(InstanceHandle handle);

private:
    std::unique_ptr<utils::ArrayAllocator> m_MeshRendererAllocator;
};
} // namespace gore::renderer