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
    cameraTransform->SetLocalPosition((gore::Vector3::Backward + gore::Vector3::Up) * 1.5f);

    gore::GameObject* gameObject = scene->NewObject();
    gameObject->SetName("TestObject");

    LOG_STREAM(INFO) << (gameObject->transform->GetLocalPosition()) << std::endl;

    TestComponent* testComponent = gameObject->AddComponent<TestComponent>();

    gameObject = scene->NewObject();
    gameObject->SetName("TestObject L");
    gameObject->transform->SetLocalPosition(gore::Vector3::Left * 2.0f);
    testComponent = gameObject->AddComponent<TestComponent>();

    gameObject = scene->NewObject();
    gameObject->SetName("TestObject R");
    gameObject->transform->SetLocalPosition(gore::Vector3::Right * 2.0f);
    testComponent = gameObject->AddComponent<TestComponent>();

    gameObject = scene->NewObject();
    gameObject->SetName("TestObject F");
    gameObject->transform->SetLocalPosition(gore::Vector3::Forward * 2.0f);
    testComponent = gameObject->AddComponent<TestComponent>();

    gameObject = scene->NewObject();
    gameObject->SetName("TestObject B");
    gameObject->transform->SetLocalPosition(gore::Vector3::Backward * 2.0f);
    testComponent = gameObject->AddComponent<TestComponent>();

    gameObject = scene->NewObject();
    gameObject->SetName("TestObject U");
    gameObject->transform->SetLocalPosition(gore::Vector3::Up * 2.0f);
    testComponent = gameObject->AddComponent<TestComponent>();

    gameObject = scene->NewObject();
    gameObject->SetName("TestObject D");
    gameObject->transform->SetLocalPosition(gore::Vector3::Down * 2.0f);
    testComponent = gameObject->AddComponent<TestComponent>();
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
