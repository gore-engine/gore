#include "GPUTransformChangeSystem.h"

#include "Object/Transform.h"

#include "Math/Matrix4x4.h"

namespace gore::renderer
{
GPUTransformChangeSystem::GPUTransformChangeSystem() noexcept :
    m_Transforms(1024, nullptr),
    m_CurMatrix(1024, Matrix4x4::Identity),
    m_PrevMatrix(1024, Matrix4x4::Identity),
    m_TransformAllocator()
{
}

void GPUTransformChangeSystem::AddTransform(Transform* transform)
{
    uint32_t index = TransformAllocator::k_InvalidIndex;
    if (m_TransformAllocator.IsFull())
    {
        IncreaseSize();
        index = m_TransformAllocator.Allocate();
    }

#if ENGINE_DEBUG
    auto iter = std::find(m_Transforms.begin(), m_Transforms.end(), transform);
    if (iter != m_Transforms.end())
    {
        LOG(ERROR, "Transform already exists in the system");
        return;
    }
#endif

    m_Transforms[index] = transform;
    m_CurMatrix[index]  = transform->GetLocalToWorldMatrix();
    m_PrevMatrix[index] = Matrix4x4::Zero;
}

void GPUTransformChangeSystem::RemoveTransform(Transform* transform)
{
    auto iter = std::find(m_Transforms.begin(), m_Transforms.end(), transform);
    if (iter == m_Transforms.end())
    {
        LOG(ERROR, "Transform does not exist in the system");
        return;
    }

    uint32_t index = static_cast<uint32_t>(std::distance(m_Transforms.begin(), m_Transforms.end()));

    m_TransformAllocator.Free(index);
    m_Transforms[index] = nullptr;
}

void GPUTransformChangeSystem::IncreaseSize()
{
    m_CurMatrix.resize(m_CurMatrix.size() + TransformAllocator::k_IncreaseSize, Matrix4x4::Identity);
    m_PrevMatrix.resize(m_PrevMatrix.size() + TransformAllocator::k_IncreaseSize, Matrix4x4::Identity);
}
} // namespace gore::renderer