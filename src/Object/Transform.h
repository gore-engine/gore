#pragma once
#include "Export.h"

#include "Component.h"

#include "Core/Log.h"

#include "Math/Types.h"

namespace gore
{

ENGINE_CLASS(Transform) final : public Component
{
public:
    NON_COPYABLE(Transform);

    explicit Transform(GameObject* gameObject)
        : Component(gameObject)
        , m_LocalPosition(Vector3::Zero)
        , m_LocalRotation(Quaternion::Identity)
        , m_LocalScale(Vector3::One)
    {}

    ~Transform() override = default;

    void Start() override;
    void Update() override;

public:
    [[nodiscard]] Vector3 GetLocalPosition() const;
    void SetLocalPosition(const Vector3& position);

    [[nodiscard]] Vector3 GetLocalScale() const;
    void SetLocalScale(const Vector3& scale);

    [[nodiscard]] Quaternion GetLocalRotation() const;
    void SetLocalRotation(const Quaternion& rotation);
    [[nodiscard]] Vector3 GetLocalEulerAngles() const;
    void SetLocalEulerAngles(const Vector3& eulerAngles);

    [[nodiscard]] Matrix4x4 GetLocalToWorldMatrix() const;

private:
    Vector3 m_LocalPosition;
    Quaternion m_LocalRotation;
    Vector3 m_LocalScale;
};

} // namespace gore