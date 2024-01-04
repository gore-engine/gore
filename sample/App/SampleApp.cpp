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
#include "Core/Log.h"

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

    gore::GameObject* gameObject = scene->NewObject();
    gameObject->SetName("TestObject");
    // This is a NO-OP because the GameObject automatically creates a Transform component when it is created.
    // It will only print an Error message to console.
    gameObject->AddComponent<gore::Transform>();

    // I don't know why but this will cause Link Error saying it cannot find the ostream& operator<< overload
    // However in Transform.cpp it works perfectly
    // TODO: Maybe Check CMake SampleApp's library linking
    // LOG_STREAM(INFO) << (gameObject->transform->GetLocalPosition()) << std::endl;

    TestComponent* testComponent = gameObject->AddComponent<TestComponent>();
}

void SampleApp::Update()
{
    float deltaTime = GetDeltaTime();
    UpdateFPSText(deltaTime);
}

void SampleApp::Render()
{
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
        ss << "SampleApp FPS: " << std::fixed << std::setprecision(2) << (float)frameCount / timer << std::endl;
        GetWindow()->SetTitle(ss.str());
        timer      = 0.0f;
        frameCount = 0;
    }
}
