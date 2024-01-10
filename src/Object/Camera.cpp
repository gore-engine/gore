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

Matrix4x4 Camera::GetProjectionMatrix() const
{
    return m_ProjectionType == ProjectionType::Perspective ?
               Matrix4x4::CreatePerspectiveFieldOfViewLH(m_FOV, m_AspectRatio, m_Near, m_Far) :
               Matrix4x4::CreateOrthographicLH(m_AspectRatio, m_AspectRatio, m_Near, m_Far);
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


} // namespace gore
