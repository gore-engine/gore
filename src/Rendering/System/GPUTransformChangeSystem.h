#pragma once

#include "Prefix.h"

#include "Math/Matrix4x4.h"

#include "Utilities/Allocator/ArrayAllocator.h"

#include <vector>

namespace gore
{
class Transform;
}

namespace gore::renderer
{
class GPUTransformChangeSystem
{
    using TransformAllocator = utils::ArrayAllocator;

public:
    GPUTransformChangeSystem() noexcept;
    ~GPUTransformChangeSystem() noexcept;

    void AddTransform(Transform* transform) noexcept;
    void RemoveTransform(Transform* transform) noexcept;

private:
    void IncreaseSize();

private:
    TransformAllocator m_TransformAllocator;

    std::vector<Transform*> m_Transforms;

    std::vector<Matrix4x4> m_CurMatrix;
    std::vector<Matrix4x4> m_PrevMatrix;
};
} // namespace gore::renderer