#pragma once

#include "Prefix.h"

#include <memory>

#include "Rendering/Handle.h"
#include "Rendering/Components/MeshRenderer.h"
#include "Utilities/Allocator/ArrayAllocator.h"

namespace gore::renderer
{
struct RendererHandle
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

private:
    uint32_t index;
};

class RendererManager
{
    SINGLETON(RendererManager)

public:
    RendererManager();

    RendererHandle GetRendererHandle();
    void FreeRendererHandle(RendererHandle handle);

private:
    std::unique_ptr<utils::ArrayAllocator> m_MeshRendererAllocator;
};
} // namespace gore::renderer