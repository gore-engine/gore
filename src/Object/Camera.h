#pragma once

#include "Math/Constants.h"
#include "Object/Component.h"

namespace gore
{

struct Matrix4x4;
class Window;

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
    [[nodiscard]] Window* GetWindow() const { return m_Window; }
    void SetWindow(Window* window) { m_Window = window; }

    [[nodiscard]] float GetPerspectiveFOV() const { return m_PerspectiveFOV; }
    void SetFOV(float fov) { m_PerspectiveFOV = fov; }

    [[nodiscard]] float GetOrthographicSize() const { return m_OrthographicSize; }
    void SetOrthographicSize(float size) { m_OrthographicSize = size; }

    [[nodiscard]] float GetNear() const { return m_Near; }
    void SetNear(float near) { m_Near = near; }
    [[nodiscard]] float GetFar() const { return m_Far; }
    void SetFar(float far) { m_Far = far; }

    [[nodiscard]] ProjectionType GetProjectionType() const { return m_ProjectionType; }
    void SetProjectionType(ProjectionType projectionType) { m_ProjectionType = projectionType; }

    [[nodiscard]] float GetPerspectiveAspectRatio() const { return m_AspectRatio; }
    void SetAspectRatio(float aspectRatio);
    void ResetAspectRatio();
    // clang-format on

public:
    NON_COPYABLE(Camera);

    Camera() = delete;
    explicit Camera(GameObject* gameObject) noexcept;
    ~Camera() override = default;

    void Start() override;
    void Update() override;

public:
    static const float DefaultAspectRatio;
    static const float DefaultPerspectiveFOV;
    static const float DefaultOrthographicSize;
    static const float DefaultNear;
    static const float DefaultFar;
    static const ProjectionType DefaultProjectionType;

private:
    enum class AspectRatioMode
    {
        FollowWindow,
        Custom
    };

private:
    Window* m_Window;

    AspectRatioMode m_AspectRatioMode;

    float m_AspectRatio;
    float m_PerspectiveFOV;
    float m_OrthographicSize;
    float m_Near;
    float m_Far;
    ProjectionType m_ProjectionType;
};

} // namespace gore