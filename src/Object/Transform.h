#pragma once
#include "Export.h"

#include "Component.h"

#include "Core/Log.h"

#include "Math/Types.h"
#include "Math/TQS.h"

#include <vector>
#include <iterator>


namespace gore
{

ENGINE_CLASS(Transform) final : public Component
{
public:
    NON_COPYABLE(Transform);

    explicit Transform(GameObject * gameObject) :
        Component(gameObject),
        m_Parent(nullptr),
        m_Children(),
        m_LocalTQS(TQS::CreateIdentity())
    {
    }

    ~Transform() override = default;

    void Start() override;
    void Update() override;

public:
    // clang-format off
    // Hierarchy
    [[nodiscard]] Transform* GetParent() const { return m_Parent; }
    void SetParent(Transform* newParent, bool reCalculateLocalTQS = true);

    [[nodiscard]] Transform* GetChild(int index) const { return m_Children[index]; }
    [[nodiscard]] int GetChildCount() const { return static_cast<int>(m_Children.size()); }

    // iterator
    [[nodiscard]] auto begin() { return m_Children.begin(); }
    [[nodiscard]] auto end() { return m_Children.end(); }
    [[nodiscard]] auto begin() const { return m_Children.cbegin(); }
    [[nodiscard]] auto end() const { return m_Children.cend(); }

    // reverse iterator
    [[nodiscard]] auto rbegin() { return m_Children.rbegin(); }
    [[nodiscard]] auto rend() { return m_Children.rend(); }
    [[nodiscard]] auto rbegin() const { return m_Children.crbegin(); }
    [[nodiscard]] auto rend() const { return m_Children.crend(); }
    // clang-format on

    [[nodiscard]] Transform* GetRoot() const;

    [[nodiscard]] Transform* Find(const std::string& name, bool recursive = false) const;

public:
    // clang-format off
    [[nodiscard]] Vector3 GetLocalPosition() const { return m_LocalTQS.t; }
    void SetLocalPosition(const Vector3& position) { m_LocalTQS.t = position; }

    [[nodiscard]] Vector3 GetLocalScale() const { return m_LocalTQS.s; }
    void SetLocalScale(const Vector3& scale) { m_LocalTQS.s = scale; }

    [[nodiscard]] Quaternion GetLocalRotation() const { return m_LocalTQS.q; }
    void SetLocalRotation(const Quaternion& rotation) { m_LocalTQS.q = rotation; }
    // clang-format on

    [[nodiscard]] Vector3 GetLocalEulerAngles() const;
    void SetLocalEulerAngles(const Vector3& eulerAngles);

    void RotateAroundAxis(const Vector3& axis, float angle);
    void RotateAroundPointInWorldSpace(const Vector3& pointInWorldSpace, const Vector3& axisInWorldSpace, float angle);

    Vector3 TransformPoint(const Vector3& point) const;
    Vector3 TransformVector3(const Vector3& direction) const;

    Vector3 InverseTransformPoint(const Vector3& point) const;
    Vector3 InverseTransformVector3(const Vector3& vector, bool useScale = true) const;

    [[nodiscard]] Matrix4x4 GetLocalToWorldMatrix() const;
    [[nodiscard]] Matrix4x4 GetLocalToWorldMatrixIgnoreScale() const;

    [[nodiscard]] Matrix4x4 GetWorldToLocalMatrix() const;
    [[nodiscard]] Matrix4x4 GetWorldToLocalMatrixIgnoreScale() const;

    [[nodiscard]] TQS GetLocalToWorldTQS() const;
    [[nodiscard]] TQS GetWorldToLocalTQS() const;

private:
    Transform* m_Parent;
    std::vector<Transform*> m_Children;

    TQS m_LocalTQS;
};

} // namespace gore