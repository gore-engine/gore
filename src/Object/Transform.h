#pragma once
#include "Export.h"

#include "Component.h"

#include "Core/Log.h"


// THIS IS IMPORTANT: DO NOT INCLUDE ANYTHING (OTHER THAN this Math.h) FROM THE ENGINE BELOW THIS LINE
// This is because the SAL annotation preprocessor (sal.h) provided by Microsft::DirectXMath conflicts with GNU C++ STL
// and the workaround is to include system headers BEFORE including DirectXMath headers
// See https://github.com/Microsoft/DirectXMath?tab=readme-ov-file#compiler-support for details
// Also, this has already an issue before: https://github.com/microsoft/vcpkg/issues/26702 
#include "Math/Math.h"

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