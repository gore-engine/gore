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
    m_Pitch(gore::math::constants::PI_4)
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

        if (m_Pitch > gore::math::constants::PI_3 * 2.0f)
            m_Pitch = gore::math::constants::PI_3 * 2.0f;

        if (m_Pitch < -gore::math::constants::PI_3 * 2.0f)
            m_Pitch = -gore::math::constants::PI_3 * 2.0f;

        if (m_Yaw > gore::math::constants::PI)
            m_Yaw -= gore::math::constants::PI * 2.0f;

        if (m_Yaw < -gore::math::constants::PI)
            m_Yaw += gore::math::constants::PI * 2.0f;

        // Considering what we really want, we should use extrinsic rotations (i.e. global rotation axes)
        gore::Quaternion yawRotation   = gore::Quaternion::CreateFromAxisAngle(gore::Vector3::Up, m_Yaw);
        gore::Quaternion pitchRotation = gore::Quaternion::CreateFromAxisAngle(gore::Vector3::Right, m_Pitch);

        transform->SetLocalRotation(yawRotation * pitchRotation);

        LOG_STREAM(DEBUG) << "Yaw: " << m_Yaw << " Pitch: " << m_Pitch << std::endl;
    }
}
