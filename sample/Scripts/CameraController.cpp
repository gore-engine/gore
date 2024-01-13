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
    m_Mouse(nullptr)
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
    gore::Vector3 up    = transform->GetLocalToWorldMatrix().GetUp();

    float speedModifier = 1.0f;
    if (m_Keyboard->KeyState(gore::KeyCode::LeftShift))
        speedModifier = 5.0f;

    transform->SetLocalPosition(transform->GetLocalPosition() +
                                deltaTime * speed * speedModifier * (m_ActionMoveHorizontal->Value() * right +
                                                                     m_ActionMoveVertical->Value() * front +
                                                                     m_ActionMoveUpDown->Value() * up));

    bool mouseRight = m_ActionAllowRotation->Held();
    m_Mouse->SetCursorShow(!mouseRight);

    float yawDelta = 0.0f;
    float pitchDelta = 0.0f;
    float rollDelta = 0.0f;

    if (m_ActionAllowRotation->Held())
    {
        float mouseSensitivity = 0.001f;
        yawDelta += m_ActionRotateHorizontal->Delta() * mouseSensitivity;
        pitchDelta += m_ActionRotateVertical->Delta() * mouseSensitivity;
    }

    if (m_Keyboard->KeyState(gore::KeyCode::R))
        rollDelta += deltaTime;
    if (m_Keyboard->KeyState(gore::KeyCode::F))
        rollDelta -= deltaTime;

    if (m_Keyboard->KeyState(gore::KeyCode::Right))
        yawDelta += deltaTime;
    if (m_Keyboard->KeyState(gore::KeyCode::Left))
        yawDelta -= deltaTime;

    if (m_Keyboard->KeyState(gore::KeyCode::Down))
        pitchDelta += deltaTime;
    if (m_Keyboard->KeyState(gore::KeyCode::Up))
        pitchDelta -= deltaTime;

    gore::Quaternion rollRotation  = gore::Quaternion::CreateFromAxisAngle(front, rollDelta);
    gore::Quaternion pitchRotation = gore::Quaternion::CreateFromAxisAngle(right, pitchDelta);
    gore::Quaternion yawRotation   = gore::Quaternion::CreateFromAxisAngle(up, yawDelta);

    gore::Quaternion rot = transform->GetLocalRotation();
    rot = rot * rollRotation * pitchRotation * yawRotation;
    transform->SetLocalRotation(rot);

    //    transform->SetLocalRotation(gore::Quaternion::CreateFromYawPitchRoll(m_Yaw, m_Roll, -m_Pitch));

    gore::Camera* camera = m_GameObject->GetComponent<gore::Camera>();
    float fov            = camera->GetPerspectiveFOV();
    fov += m_ActionZoom->Delta() * -0.1f;
    if (fov < 0.1f)
        fov = 0.1f;
    if (fov > gore::math::constants::PI - 0.1f)
        fov = gore::math::constants::PI - 0.1f;
    camera->SetFOV(fov);
}
