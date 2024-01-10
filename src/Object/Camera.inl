float Camera::GetFOV() const
{
    return m_FOV;
}

void Camera::SetFOV(float fov)
{
    m_FOV = fov;
}

float Camera::GetNear() const
{
    return m_Near;
}

void Camera::SetNear(float near)
{
    m_Near = near;
}

float Camera::GetFar() const
{
    return m_Far;
}

void Camera::SetFar(float far)
{
    m_Far = far;
}

Camera::ProjectionType Camera::GetProjectionType() const
{
    return m_ProjectionType;
}

void Camera::SetProjectionType(gore::Camera::ProjectionType projectionType)
{
    m_ProjectionType = projectionType;
}
