#pragma once

#include "Object/Component.h"

namespace gore
{

struct Matrix4x4;

ENGINE_CLASS(Camera) final : public Component
{
public:
    enum class ProjectionType
    {
        Perspective,
        Orthographic
    };

public:
    // common matrices
    [[nodiscard]] Matrix4x4 GetProjectionMatrix() const;
    [[nodiscard]] Matrix4x4 GetViewMatrix() const;
    [[nodiscard]] Matrix4x4 GetViewProjectionMatrix() const;

    // properties
    [[nodiscard]] float GetFOV() const;
    void SetFOV(float fov);
    [[nodiscard]] float GetNear() const;
    void SetNear(float near);
    [[nodiscard]] float GetFar() const;
    void SetFar(float far);
    [[nodiscard]] ProjectionType GetProjectionType() const;
    void SetProjectionType(ProjectionType projectionType);

public:
    NON_COPYABLE(Camera);

    explicit Camera(GameObject * gameObject, ProjectionType projectionType = ProjectionType::Perspective, float fov = 60.0f, float AspectRatio = 16.0f / 9.0f, float near = 0.1f, float far = 1000.0f) :
        Component(gameObject),
        m_FOV(fov),
        m_AspectRatio(AspectRatio),
        m_Near(near),
        m_Far(far),
        m_ProjectionType(projectionType)

    {
    }

    ~Camera() override = default;

    void Start() override;
    void Update() override;

private:
    float m_FOV;
    float m_AspectRatio;
    float m_Near;
    float m_Far;
    ProjectionType m_ProjectionType;
};

#include "Camera.inl"

} // namespace gore