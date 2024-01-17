#include "TestComponent.h"
#include "Object/GameObject.h"
#include "Object/Camera.h"
#include "Core/Log.h"
#include "Core/Time.h"
#include "Math/Constants.h"
#include "Input/InputSystem.h"

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

    m_Keyboard = inputSystem->GetKeyboard();
    m_Mouse    = inputSystem->GetMouse();

    m_ActionMoveHorizontal = m_Keyboard->RegisterAction("MoveHorizontal", gore::KeyCode::D, gore::KeyCode::A);
    m_ActionMoveVertical   = m_Keyboard->RegisterAction("MoveVertical", gore::KeyCode::W, gore::KeyCode::S);
    m_ActionMoveUpDown     = m_Keyboard->RegisterAction("MoveUpDown", gore::KeyCode::E, gore::KeyCode::Q);

    m_ActionAllowRotation = m_Mouse->RegisterAction("AllowRotation", gore::MouseButtonCode::Right);

    m_ActionRotateHorizontal = m_Mouse->RegisterAction("RotateHorizontal", gore::MouseMovementCode::X);
    m_ActionRotateVertical   = m_Mouse->RegisterAction("RotateVertical", gore::MouseMovementCode::Y);
    m_ActionZoom             = m_Mouse->RegisterAction("Zoom", gore::MouseMovementCode::ScrollY);
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

    float speedModifier = 1.0f;
    if (m_Keyboard->KeyState(gore::KeyCode::LeftShift))
        speedModifier = 5.0f;

    transform->SetLocalPosition(transform->GetLocalPosition() +
                                deltaTime * speed * speedModifier * (m_ActionMoveHorizontal->Value() * right +
                                                                     m_ActionMoveVertical->Value() * front +
                                                                     m_ActionMoveUpDown->Value() * up));

    bool mouseRight = m_ActionAllowRotation->Held();
    m_Mouse->SetCursorShow(!mouseRight);

    if (m_ActionAllowRotation->Held())
    {
        float mouseSensitivity = 0.001f;
        m_Yaw += m_ActionRotateHorizontal->Delta() * mouseSensitivity;
        m_Pitch += m_ActionRotateVertical->Delta() * mouseSensitivity;
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

    // gore::Quaternion rollRotation  = gore::Quaternion::CreateFromAxisAngle(gore::Vector3::Forward, m_Roll);
    // gore::Quaternion pitchRotation = gore::Quaternion::CreateFromAxisAngle(gore::Vector3::Right, m_Pitch);
    // gore::Quaternion yawRotation   = gore::Quaternion::CreateFromAxisAngle(gore::Vector3::Up, m_Yaw);

    // the two ways setting rotations are different because of the rotation order. Axes are the same.
    // transform->SetLocalRotation(rollRotation * pitchRotation * yawRotation);
    // I believe rtm::quat_from_euler_angles is using XYZ rotation order:
    // because when using this function, the yaw and pitch are always rotated as expected and not affected by the roll,
    // which means roll(i.e. Z) is the last rotation.
    transform->SetLocalRotation(gore::Quaternion::CreateFromYawPitchRoll(m_Yaw, m_Pitch, m_Roll));

    gore::Camera* camera = m_GameObject->GetComponent<gore::Camera>();
    float fov = camera->GetPerspectiveFOV();
    fov += m_ActionZoom->Delta() * -0.1f;
    if (fov < 0.1f)
        fov = 0.1f;
    if (fov > gore::math::constants::PI - 0.1f)
        fov = gore::math::constants::PI - 0.1f;
    camera->SetFOV(fov);
}
