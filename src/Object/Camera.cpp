#include "Camera.h"

#include "Object/GameObject.h"
#include "Math/Matrix4x4.h"
#include "Windowing/Window.h"
#include "Core/App.h"

namespace gore
{

Camera* Camera::Main = nullptr;
std::vector<Camera*> Camera::s_All;

void Camera::Start()
{
}

void Camera::Update()
{
    if (m_AspectRatioMode == AspectRatioMode::FollowWindow)
    {
        ResetAspectRatio();
    }
}

Camera::Camera(gore::GameObject* gameObject) noexcept :
    Component(gameObject),
    m_Window(App::Get()->GetWindow()),
    m_AspectRatioMode(AspectRatioMode::FollowWindow),
    m_ProjectionType(DefaultProjectionType),
    m_AspectRatio(DefaultAspectRatio),
    m_PerspectiveFOV(DefaultPerspectiveFOV),
    m_OrthographicSize(DefaultOrthographicSize),
    m_Near(DefaultNear),
    m_Far(DefaultFar)
{
    if (Main == nullptr)
    {
        Main = this;
    }

    s_All.push_back(this);
}

Camera::~Camera()
{
    s_All.erase(std::remove(s_All.begin(), s_All.end(), this), s_All.end());

    if (Main == this)
    {
        Main = nullptr;
    }

    if (s_All.empty() == false)
    {
        Main = s_All.front();
    }
}

Matrix4x4 Camera::GetProjectionMatrix() const
{
    return m_ProjectionType == ProjectionType::Perspective ?
               Matrix4x4::CreatePerspectiveFieldOfViewLH(m_PerspectiveFOV, m_AspectRatio, m_Near, m_Far) :
               Matrix4x4::CreateOrthographicLH(m_OrthographicSize * m_AspectRatio, m_OrthographicSize, m_Near, m_Far);
}

Matrix4x4 Camera::GetViewMatrix() const
{
    // 1. A camera is always attached to a game object, so we can safely assume that the transform is not null
    // 2. A camera never cares about the scale of the transform, so we can safely ignore it
    return m_GameObject->GetTransform()->GetWorldToLocalMatrixIgnoreScale();
}

Matrix4x4 Camera::GetViewProjectionMatrix() const
{
    return GetViewMatrix() * GetProjectionMatrix();
}

void Camera::SetAspectRatio(float aspectRatio)
{
    m_AspectRatio = aspectRatio;
    m_AspectRatioMode = AspectRatioMode::Custom;
}

void Camera::ResetAspectRatio()
{
    int width, height;
    m_Window->GetSize(&width, &height);
    m_AspectRatio = static_cast<float>(width) / static_cast<float>(height);
    m_AspectRatioMode = AspectRatioMode::FollowWindow;
}

// constants
const Camera::ProjectionType Camera::DefaultProjectionType = Camera::ProjectionType::Perspective;
const float Camera::DefaultAspectRatio                     = 16.0f / 9.0f;
const float Camera::DefaultPerspectiveFOV                  = math::constants::PI_3;
const float Camera::DefaultOrthographicSize                = 1.0f;
const float Camera::DefaultNear                            = 0.1f;
const float Camera::DefaultFar                             = 1000.0f;

Camera* Camera::FindMainCamera()
{
    if (Main == nullptr)
    {
        if (s_All.empty() == false)
        {
            Main = s_All.front();
        }
    }

    return Main;
}

} // namespace gore
