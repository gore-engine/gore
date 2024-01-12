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
    m_PerspectiveFOV(DefaultPerspectiveFOV),
    m_PerspectiveAspectRatio(DefaultPerspectiveAspectRatio),
    m_OrthographicViewWidth(DefaultOrthographicViewWidth),
    m_OrthographicViewHeight(DefaultOrthographicViewHeight),
    m_ZMin(DefaultZMin),
    m_ZMax(DefaultZMax)
{
}

Matrix4x4 Camera::GetProjectionMatrix() const
{
    return m_ProjectionType == ProjectionType::Perspective ?
               Matrix4x4::CreatePerspectiveFieldOfViewLH(m_PerspectiveFOV, m_PerspectiveAspectRatio, m_ZMin, m_ZMax) :
               Matrix4x4::CreateOrthographicLH(m_OrthographicViewWidth, m_OrthographicViewHeight, m_ZMin, m_ZMax);
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
const float Camera::DefaultPerspectiveFOV                  = math::constants::PI_3;
const float Camera::DefaultPerspectiveAspectRatio          = 16.0f / 9.0f;
const float Camera::DefaultOrthographicViewWidth           = 16.0f;
const float Camera::DefaultOrthographicViewHeight          = 9.0f;
const float Camera::DefaultZMin                            = 0.1f;
const float Camera::DefaultZMax                            = 1000.0f;

} // namespace gore
