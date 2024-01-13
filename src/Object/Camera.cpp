#include "Camera.h"

#include "Object/GameObject.h"
#include "Math/Matrix4x4.h"

namespace gore
{

void Camera::Start()
{
}

void Camera::Update()
{
}

Camera::Camera(gore::GameObject* gameObject) noexcept :
    Component(gameObject),
    m_ProjectionType(DefaultProjectionType),
    m_AspectRatio(DefaultAspectRatio),
    m_PerspectiveFOV(DefaultPerspectiveFOV),
    m_OrthographicSize(DefaultOrthographicSize),
    m_Near(DefaultNear),
    m_Far(DefaultFar)
{
}

Matrix4x4 Camera::GetProjectionMatrix() const
{
    return m_ProjectionType == ProjectionType::Perspective ?
               Matrix4x4::CreatePerspectiveFieldOfViewLH(m_PerspectiveFOV, m_AspectRatio, m_Near, m_Far) :
               Matrix4x4::CreateOrthographicLH(DefaultOrthographicSize * DefaultAspectRatio, DefaultOrthographicSize, m_Near, m_Far);
}

Matrix4x4 Camera::GetViewMatrix() const
{
    // 1. A camera is always attached to a game object, so we can safely assume that the transform is not null
    // 2. A camera never cares about the scale of the transform, so we can safely ignore it
    return m_GameObject->transform->GetWorldToLocalMatrixIgnoreScale();
}

Matrix4x4 Camera::GetViewProjectionMatrix() const
{
    return GetViewMatrix() * GetProjectionMatrix();
}

// constants
const Camera::ProjectionType Camera::DefaultProjectionType = Camera::ProjectionType::Perspective;
const float Camera::DefaultAspectRatio                     = 16.0f / 9.0f;
const float Camera::DefaultPerspectiveFOV                  = math::constants::PI_3;
const float Camera::DefaultOrthographicSize                = 1.0f;
const float Camera::DefaultNear                            = 0.1f;
const float Camera::DefaultFar                             = 1000.0f;

} // namespace gore
