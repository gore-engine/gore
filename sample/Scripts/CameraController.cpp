#include "TestComponent.h"
#include "Object/GameObject.h"
#include "Core/Log.h"
#include "Core/Time.h"
#include "Math/Constants.h"

#include "CameraController.h"

CameraController::CameraController(gore::GameObject* gameObject) :
    Component(gameObject),
    m_Keyboard(nullptr),
    m_Mouse(nullptr),
    m_Yaw(0.0f),
    m_Pitch(gore::math::constants::PI_4),
    m_Roll(0.0f)
{
}

CameraController::~CameraController()
{
}

void CameraController::Start()
{
    const gore::InputSystem* inputSystem = gore::InputSystem::Get();
    m_Keyboard                           = inputSystem->GetKeyboard();
    m_Mouse                              = inputSystem->GetMouse();
}

void CameraController::Update()
{
    const float speed = 5.0f;

    float deltaTime = GetDeltaTime();
    float totalTime = GetTotalTime();

    gore::Transform* transform = m_GameObject->GetComponent<gore::Transform>();

    gore::Vector3 front = transform->GetLocalToWorldMatrix().GetForward();
    gore::Vector3 right = transform->GetLocalToWorldMatrix().GetRight();
    gore::Vector3 up    = gore::Vector3::Up;

    right = up.Cross(front).Normalized();

    if (m_Keyboard->KeyState(gore::KeyCode::W))
        transform->SetLocalPosition(transform->GetLocalPosition() + front * deltaTime * speed);
    if (m_Keyboard->KeyState(gore::KeyCode::S))
        transform->SetLocalPosition(transform->GetLocalPosition() - front * deltaTime * speed);

    if (m_Keyboard->KeyState(gore::KeyCode::A))
        transform->SetLocalPosition(transform->GetLocalPosition() - right * deltaTime * speed);
    if (m_Keyboard->KeyState(gore::KeyCode::D))
        transform->SetLocalPosition(transform->GetLocalPosition() + right * deltaTime * speed);

    if (m_Keyboard->KeyState(gore::KeyCode::Q))
        transform->SetLocalPosition(transform->GetLocalPosition() - up * deltaTime * speed);
    if (m_Keyboard->KeyState(gore::KeyCode::E))
        transform->SetLocalPosition(transform->GetLocalPosition() + up * deltaTime * speed);

    bool mouseRight = m_Mouse->ButtonState(gore::MouseButtonCode::Right);
    m_Mouse->SetCursorShow(!mouseRight);

    if (mouseRight)
    {
        float mouseSensitivity = 0.001f;
        m_Yaw += m_Mouse->GetDelta(gore::MouseMovementCode::X) * mouseSensitivity;
        m_Pitch += m_Mouse->GetDelta(gore::MouseMovementCode::Y) * mouseSensitivity;
    }

    if (m_Keyboard->KeyState(gore::KeyCode::R))
        m_Roll += deltaTime;
    if (m_Keyboard->KeyState(gore::KeyCode::F))
        m_Roll -= deltaTime;

    if (m_Keyboard->KeyState(gore::KeyCode::Right))
        m_Yaw += deltaTime;
    if (m_Keyboard->KeyState(gore::KeyCode::Left))
        m_Yaw -= deltaTime;

    if (m_Keyboard->KeyState(gore::KeyCode::Down))
        m_Pitch += deltaTime;
    if (m_Keyboard->KeyState(gore::KeyCode::Up))
        m_Pitch -= deltaTime;

    if (m_Pitch > gore::math::constants::PI_3)
        m_Pitch = gore::math::constants::PI_3;

    if (m_Pitch < -gore::math::constants::PI_3)
        m_Pitch = -gore::math::constants::PI_3;

    gore::Quaternion rollRotation  = gore::Quaternion::CreateFromAxisAngle(front, m_Roll);
    gore::Quaternion pitchRotation = gore::Quaternion::CreateFromAxisAngle(right, m_Pitch);
    gore::Quaternion yawRotation   = gore::Quaternion::CreateFromAxisAngle(gore::Vector3::Up, m_Yaw);

    transform->SetLocalRotation(yawRotation * pitchRotation * rollRotation);

//    transform->SetLocalRotation(gore::Quaternion::CreateFromYawPitchRoll(m_Yaw, m_Roll, -m_Pitch));
}
