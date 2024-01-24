#include "SampleApp.h"

#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>

#include "Core/Time.h"
#include "Windowing/Window.h"
#include "Scene/Scene.h"
#include "Object/GameObject.h"
#include "Object/Transform.h"
#include "Object/Camera.h"
#include "Core/Log.h"
#include "Math/Constants.h"

#include "Scripts/TestComponent.h"
#include "Scripts/CameraController.h"
#include "Scripts/PeriodicallySwitchParent.h"
#include "Scripts/SelfRotate.h"
#include "Scripts/SelfMoveBackAndForth.h"
#include "Scripts/SelfScaleInBetweenRange.h"
#include "Scripts/PeriodicallyChangeWorldTRS.h"
#include "Scripts/SelfDestroyAfterSeconds.h"
#include "Scripts/DeleteMultipleGameObjectsAfterSeconds.h"

SampleApp::SampleApp(int argc, char** argv) :
    App(argc, argv)
{
}

SampleApp::~SampleApp()
{
}

void SampleApp::Initialize()
{
    // gore::Logger::Default().SetLevel(gore::LogLevel::DEBUG);

    scene = new gore::Scene("MainScene");

    gore::GameObject* cameraGameObject = scene->NewObject();
    cameraGameObject->SetName("MainCamera");
    gore::Camera* camera = cameraGameObject->AddComponent<gore::Camera>();
    cameraGameObject->AddComponent<CameraController>();

    gore::Transform* cameraTransform = cameraGameObject->GetComponent<gore::Transform>();
    cameraTransform->RotateAroundAxis(gore::Vector3::Right, gore::math::constants::PI_4);
    cameraTransform->SetLocalPosition((gore::Vector3::Backward + gore::Vector3::Up) * 7.5f);

    gore::GameObject* gameObject = scene->NewObject();
    gameObject->SetName("TestObject O, T&R&S");

    auto pSelfRotate           = gameObject->AddComponent<SelfRotate>();
    auto pSelfScale            = gameObject->AddComponent<SelfScaleInBetweenRange>();
    auto pSelfMoveBackAndForth = gameObject->AddComponent<SelfMoveBackAndForth>();

    const float distance = 2.5f;

    gameObject = scene->NewObject();
    gameObject->SetName("TestObject L, T&S");
    gameObject->GetTransform()->SetLocalPosition(gore::Vector3::Left * distance);
    pSelfScale = gameObject->AddComponent<SelfScaleInBetweenRange>();
    pSelfScale->SetMinMaxScale(0.5f, 1.5f);
    pSelfMoveBackAndForth              = gameObject->AddComponent<SelfMoveBackAndForth>();
    pSelfMoveBackAndForth->m_Direction = gore::Vector3::Left;
    auto pLeftObject                   = gameObject;

    gameObject = scene->NewObject();
    gameObject->SetName("TestObject R, R&S");
    gameObject->GetTransform()->SetLocalPosition(gore::Vector3::Right * distance);
    pSelfRotate               = gameObject->AddComponent<SelfRotate>();
    pSelfRotate->m_RotateAxis = gore::Vector3::Right;
    pSelfScale                = gameObject->AddComponent<SelfScaleInBetweenRange>();
    pSelfScale->SetMinMaxScale(0.5f, 1.5f);
    auto pRightObject = gameObject;

    gameObject = scene->NewObject();
    gameObject->SetName("TestObject F, T&R");
    gameObject->GetTransform()->SetLocalPosition(gore::Vector3::Forward * distance);
    pSelfRotate                        = gameObject->AddComponent<SelfRotate>();
    pSelfRotate->m_RotateAxis          = gore::Vector3::Forward;
    pSelfMoveBackAndForth              = gameObject->AddComponent<SelfMoveBackAndForth>();
    pSelfMoveBackAndForth->m_Direction = gore::Vector3::Forward;
    auto pForwardObject                = gameObject;

    gameObject = scene->NewObject();
    gameObject->SetName("TestObject B, Translation only");
    gameObject->GetTransform()->SetLocalPosition(gore::Vector3::Backward * distance);
    pSelfMoveBackAndForth              = gameObject->AddComponent<SelfMoveBackAndForth>();
    pSelfMoveBackAndForth->m_Direction = gore::Vector3::Backward;
    auto pBackwardObject               = gameObject;

    gameObject = scene->NewObject();
    gameObject->SetName("TestObject U, Rotation only");
    gameObject->GetTransform()->SetLocalPosition(gore::Vector3::Up * distance);
    pSelfRotate               = gameObject->AddComponent<SelfRotate>();
    pSelfRotate->m_RotateAxis = gore::Vector3::Up;
    auto pUpObject            = gameObject;

    gameObject = scene->NewObject();
    gameObject->SetName("TestObject D, Scale only");
    gameObject->GetTransform()->SetLocalPosition(gore::Vector3::Down * distance);
    pSelfScale = gameObject->AddComponent<SelfScaleInBetweenRange>();
    pSelfScale->SetMinMaxScale(0.5f, 1.5f);
    auto pDownObject = gameObject;

    pDownObject->GetTransform()->SetLocalScale(gore::Vector3::One * 1.2f);

    gore::GameObject* childGameObject = scene->NewObject();
    childGameObject->SetName("ChildObject");
    childGameObject->GetTransform()->SetParent(pUpObject->GetTransform());
    //    childGameObject->GetTransform()->SetParent(nullptr);
    childGameObject->GetTransform()->SetLocalPosition(gore::Vector3::Up * 1.0f);
    childGameObject->GetTransform()->SetLocalScale(gore::Vector3::One * 0.5f);

    auto grandChildGameObject = scene->NewObject();
    grandChildGameObject->SetName("GrandChildObject");
    grandChildGameObject->GetTransform()->SetParent(childGameObject->GetTransform());
    grandChildGameObject->GetTransform()->SetLocalPosition((gore::Vector3::Forward + gore::Vector3::Right) * 1.5f);
    grandChildGameObject->GetTransform()->SetLocalScale(gore::Vector3::One * 0.7f);

    auto pPeriodicallySwitchParent = grandChildGameObject->AddComponent<PeriodicallySwitchParent>();
    pPeriodicallySwitchParent->SetParentAB(childGameObject->GetTransform(), pLeftObject->GetTransform());
    pPeriodicallySwitchParent->m_RecalculateLocalPosition = true;

    pPeriodicallySwitchParent = childGameObject->AddComponent<PeriodicallySwitchParent>();
    pPeriodicallySwitchParent->SetParentAB(pUpObject->GetTransform(), childGameObject->GetTransform()->GetParent());
    pPeriodicallySwitchParent->m_RecalculateLocalPosition = true;

    auto pPeriodicallyChangeWorldTRS      = childGameObject->AddComponent<PeriodicallyChangeWorldTRS>();
    pPeriodicallyChangeWorldTRS->m_Period = 0.5f;
    pSelfScale                            = childGameObject->AddComponent<SelfScaleInBetweenRange>();
    pSelfScale->SetMinMaxScale(0.5f, 1.5f);

    childGameObject->AddComponent<SelfDestroyAfterSeconds>();

    auto pLeftChildObject = scene->NewObject();
    pLeftChildObject->SetName("LeftChildObject");
    pLeftChildObject->GetTransform()->SetParent(pLeftObject->GetTransform());
    pLeftChildObject->GetTransform()->SetLocalPosition(gore::Vector3::Left * 1.0f);
    pLeftChildObject->GetTransform()->SetLocalScale(gore::Vector3::One * 0.5f);

    auto pLeftGrandChildObject = scene->NewObject();
    pLeftGrandChildObject->SetName("LeftGrandChildObject");
    pLeftGrandChildObject->GetTransform()->SetParent(pLeftChildObject->GetTransform());
    pLeftGrandChildObject->GetTransform()->SetLocalPosition((gore::Vector3::Forward + gore::Vector3::Left) * 1.5f);
    pLeftGrandChildObject->GetTransform()->SetLocalScale(gore::Vector3::One * 0.7f);

    std::vector<gore::GameObject*> destroyList = {pLeftObject, pRightObject, pLeftGrandChildObject};
    auto pDeleteMultipleGameObjectsAfterSeconds =
        cameraGameObject->AddComponent<DeleteMultipleGameObjectsAfterSeconds>();
    pDeleteMultipleGameObjectsAfterSeconds->SetGameObjectsToDelete(destroyList, 3, 2.0f);

    LOG_STREAM(DEBUG) << "Find ChildObject in pUpObject at Initialization, non-recursively: "
                      << pUpObject->GetTransform()->Find("GrandChildObject")
                      << std::endl;
    LOG_STREAM(DEBUG) << "Find ChildObject in pUpObject at Initialization, recursively: "
                      << pUpObject->GetTransform()->Find("GrandChildObject", true)
                      << std::endl;
    LOG_STREAM(DEBUG) << "GetRoot() of grandChildGameObject: "
                      << grandChildGameObject->GetTransform()->GetRoot()->GetGameObject()->GetName()
                      << std::endl;
}

void SampleApp::Update()
{
    float deltaTime = GetDeltaTime();
    UpdateFPSText(deltaTime);
}

void SampleApp::Shutdown()
{
    delete scene;
}

void SampleApp::UpdateFPSText(float deltaTime)
{
    static float timer    = 0.0f;
    static int frameCount = 0;

    timer += deltaTime;
    ++frameCount;
    if (timer >= 0.5f)
    {
        std::stringstream ss;
        ss << "SampleApp FPS: " << std::fixed << std::setprecision(2) << (float)frameCount / timer << std::flush;
        GetWindow()->SetTitle(ss.str());
        timer      = 0.0f;
        frameCount = 0;
    }
}
