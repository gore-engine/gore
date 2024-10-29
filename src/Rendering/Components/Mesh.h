#pragma once

#include "Prefix.h"
#include "Export.h"

#include "Rendering/Buffer.h"
#include "Rendering/DynamicBuffer.h"

namespace gore::renderer
{
using namespace gore::gfx;

ENGINE_CLASS(Mesh) final
{
public:
    NON_COPYABLE(Mesh)

    explicit Mesh() noexcept;
    ~Mesh() = default;

    GETTER_SETTER(BufferHandle, VertexBuffer)
    GETTER_SETTER(uint32_t, VertexCount)
    GETTER_SETTER(uint32_t, VertexOffset)

    GETTER_SETTER(BufferHandle, IndexBuffer)
    GETTER_SETTER(uint32_t, IndexCount)
    GETTER_SETTER(uint32_t, IndexOffset)

private:
    BufferHandle m_VertexBuffer;
    uint32_t m_VertexCount;
    uint32_t m_VertexOffset;

    BufferHandle m_IndexBuffer;
    uint32_t m_IndexCount;
    uint32_t m_IndexOffset;
};
} // namespace gore::renderer
