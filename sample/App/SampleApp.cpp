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

    gore::GameObject* camera = scene->NewObject();
    camera->SetName("MainCamera");
    camera->AddComponent<gore::Camera>();

    gore::Transform* cameraTransform = camera->GetComponent<gore::Transform>();
    cameraTransform->RotateAroundAxis(gore::Vector3::Right, gore::math::constants::PI_4);
    cameraTransform->SetLocalPosition((gore::Vector3::Backward + gore::Vector3::Up) * 2.0f);

    gore::GameObject* gameObject = scene->NewObject();
    gameObject->SetName("TestObject");

    LOG_STREAM(INFO) << (gameObject->transform->GetLocalPosition()) << std::endl;

    TestComponent* testComponent = gameObject->AddComponent<TestComponent>();
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
