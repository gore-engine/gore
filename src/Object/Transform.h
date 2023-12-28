#pragma once
#include "Export.h"

#include "Math/Math.h"

#include "Component.h"

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
    Vector3 GetLocalPosition() const { return m_LocalPosition; }
    void SetLocalPosition(const Vector3& position);

    Vector3 GetLocalScale() const { return m_LocalScale; }
    void SetLocalScale(const Vector3& scale);

    Quaternion GetLocalRotation() const { return m_LocalRotation; }
    void SetLocalRotation(const Quaternion& rotation);
    Vector3 GetLocalEulerAngles() const { return m_LocalRotation.ToEuler(); }
    void SetLocalEulerAngles(const Vector3& eulerAngles);

private:
    Vector3 m_LocalPosition;
    Quaternion m_LocalRotation;
    Vector3 m_LocalScale;
};

} // namespace gore