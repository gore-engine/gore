#pragma once

#include "Math/Constants.h"
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

    // clang-format off
    // properties
    [[nodiscard]] float GetPerspectiveFOV() const { return m_PerspectiveFOV; }
    void SetFOV(float fov) { m_PerspectiveFOV = fov; }
    [[nodiscard]] float GetPerspectiveAspectRatio() const { return m_PerspectiveAspectRatio; }
    void SetAspectRatio(float aspectRatio) { m_PerspectiveAspectRatio = aspectRatio; }

    [[nodiscard]] float GetOrthographicWidth() const { return m_OrthographicViewWidth; }
    void SetOrthographicWidth(float width) { m_OrthographicViewWidth = width; }
    [[nodiscard]] float GetOrthographicHeight() const { return m_OrthographicViewHeight; }
    void SetOrthographicHeight(float height) { m_OrthographicViewHeight = height; }

    // okay lets avoid using near/far because of macros defined in windows.h
    [[nodiscard]] float GetZMin() const { return m_ZMin; }
    void SetNear(float zMin) { m_ZMin = zMin; }
    [[nodiscard]] float GetZMax() const { return m_ZMax; }
    void SetZMax(float zMax) { m_ZMax = zMax; }

    [[nodiscard]] ProjectionType GetProjectionType() const { return m_ProjectionType; }
    void SetProjectionType(ProjectionType projectionType) { m_ProjectionType = projectionType; }
    // clang-format on

public:
    NON_COPYABLE(Camera);

    Camera() = delete;
    explicit Camera(GameObject * gameObject) noexcept;
    ~Camera() override = default;

    void Start() override;
    void Update() override;

public:
    static const float DefaultPerspectiveFOV;
    static const float DefaultPerspectiveAspectRatio;
    static const float DefaultOrthographicViewWidth;
    static const float DefaultOrthographicViewHeight;
    static const float DefaultZMin;
    static const float DefaultZMax;
    static const ProjectionType DefaultProjectionType;

private:
    float m_PerspectiveFOV;
    float m_PerspectiveAspectRatio;
    float m_OrthographicViewWidth;
    float m_OrthographicViewHeight;
    float m_ZMin;
    float m_ZMax;
    ProjectionType m_ProjectionType;
};

#include "Camera.inl"

} // namespace gore