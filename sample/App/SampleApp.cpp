#include "SampleApp.h"

#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>

#include "Rendering/RenderSystem.h"
#include "Rendering/RenderContext.h"

#include "Rendering/AutoRenderPass.h"

#include "Core/Time.h"
#include "Windowing/Window.h"
#include "Scene/Scene.h"
#include "Object/GameObject.h"
#include "Object/Transform.h"
#include "Object/Camera.h"
#include "Core/Log.h"
#include "Math/Constants.h"

#include "Rendering/Components/Light.h"

#include "Scripts/Utils/GraphicsUtils.h"

#include "Scripts/TestComponent.h"
#include "Scripts/CameraController.h"
#include "Scripts/PeriodicallySwitchParent.h"
#include "Scripts/SelfRotate.h"
#include "Scripts/SelfMoveBackAndForth.h"
#include "Scripts/SelfScaleInBetweenRange.h"
#include "Scripts/PeriodicallyChangeWorldTRS.h"
#include "Scripts/SelfDestroyAfterSeconds.h"
#include "Scripts/DeleteMultipleGameObjectsAfterSeconds.h"

#include "Scripts/Math/BitUtils.h"

#include "Scripts/Rendering/PerframeData.h"
#include "Scripts/Rendering/PerDrawData.h"

SampleApp::SampleApp(int argc, char** argv) :
    App(argc, argv)
{
}

SampleApp::~SampleApp()
{
}

void SampleApp::InitializeRpsSystem()
{
    RpsRenderGraph& renderGraph = *m_RenderSystem->GetRpsSystem()->rpsRDG;
    // AssertIfRpsFailed(rpsProgramBindNode(rpsRenderGraphGetMainEntry(renderGraph), "Triangle", &DrawTriangleWithRPSWrapper, this));
    AssertIfRpsFailed(rpsProgramBindNode(rpsRenderGraphGetMainEntry(renderGraph), "Shadowmap", &ShadowmapPassWithRPSWrapper, this));
    AssertIfRpsFailed(rpsProgramBindNode(rpsRenderGraphGetMainEntry(renderGraph), "ForwardOpaque", &ForwardOpaquePassWithRPSWrapper, this));
}

void SampleApp::CreateRenderPassDesc()
{
    renderPasses.forwardPassDesc = {{GraphicsFormat::BGRA8_SRGB}};
    renderPasses.shadowPassDesc  = {{}, GraphicsFormat::D32_FLOAT};
}

void SampleApp::CreateUnifiedGlobalDynamicBuffer()
{
    auto& renderContext = m_RenderSystem->GetRenderContext();

    size_t alignmentSize = MathUtils::AlignUp(sizeof(PerDrawData), m_GraphicsCaps.minUniformBufferOffsetAlignment);

    size_t renderCount = 4;
    std::vector<uint8_t> dynamicUniformBufferData(alignmentSize * renderCount);

    for (size_t i = 0; i < renderCount; ++i)
    {
        PerDrawData* perDrawData = reinterpret_cast<PerDrawData*>(dynamicUniformBufferData.data() + (i * alignmentSize));
        perDrawData->model[0]       = Matrix4x4(1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, i, 0.f, 0.f, 1.f);
    }

    m_UnifiedDynamicBuffer = renderContext.CreateBuffer(
        {.debugName = "Dynamic Uniform Buffer",
         .byteSize  = static_cast<uint32_t>(dynamicUniformBufferData.size()),
         .usage     = BufferUsage::Uniform,
         .memUsage  = MemoryUsage::GPU,
         .data      = dynamicUniformBufferData.data()});

    m_UnifiedDynamicBufferHandle = renderContext.CreateDynamicBuffer(
        {.debugName = "Dynamic Uniform Buffer",
         .buffer    = m_UnifiedDynamicBuffer,
         .offset    = 0,
         .range     = sizeof(PerDrawData)});
}

void SampleApp::CreatePipelines()
{
    CreateForwardPipeline();
    CreateShadowmapPipeline();
}

void SampleApp::CreateDefaultResources()
{
    using namespace gore::gfx;
    
    RenderContext& renderContext = m_RenderSystem->GetRenderContext();

    std::vector<uint8_t> blackTextureData(4, 0);
    std::vector<uint8_t> whiteTextureData(4, 255);

    defaultResources.blackTexture = renderContext.CreateTextureHandle(
        TextureDesc
        {
            .debugName = "Black Texture",
            .width     = 1,
            .height    = 1,
            .data      = blackTextureData.data(),
            .dataSize  = 4,
        });

    defaultResources.whiteTexture = renderContext.CreateTextureHandle(
        TextureDesc
        {
            .debugName = "White Texture",
            .width     = 1,
            .height    = 1,
            .data      = whiteTextureData.data(),
            .dataSize  = 4,
        });
}

void SampleApp::CreateForwardPipeline()
{
    using namespace gore::gfx;

    RenderContext& renderContext = m_RenderSystem->GetRenderContext();
    AutoRenderPass forwardPass(&renderContext, renderPasses.forwardPassDesc);

    // Create a pipeline for the forward rendering
    std::vector<char> vertexShaderBytecode   = sample::utils::LoadShaderBytecode("sample/UnLit", ShaderStage::Vertex, "main");
    std::vector<char> fragmentShaderBytecode = sample::utils::LoadShaderBytecode("sample/UnLit", ShaderStage::Fragment, "main");

    pipelines.forwardPipeline = renderContext.CreateGraphicsPipeline(
        GraphicsPipelineDesc{
            .debugName = "UnLit Pipeline",
            .VS{
                .byteCode  = reinterpret_cast<uint8_t*>(vertexShaderBytecode.data()),
                .byteSize  = static_cast<uint32_t>(vertexShaderBytecode.size()),
                .entryFunc = "vs"},
            .PS{
                .byteCode  = reinterpret_cast<uint8_t*>(fragmentShaderBytecode.data()),
                .byteSize  = static_cast<uint32_t>(fragmentShaderBytecode.size()),
                .entryFunc = "ps"},
            .colorFormats  = {GraphicsFormat::BGRA8_SRGB},
            .depthFormat   = GraphicsFormat::D32_FLOAT,
            .stencilFormat = GraphicsFormat::Undefined,
            .vertexBufferBindings{
                {.byteStride = sizeof(Vector3) + sizeof(Vector2) + sizeof(Vector3),
                 .attributes =
                     {
                         {.byteOffset = 0, .format = GraphicsFormat::RGB32_FLOAT},
                         {.byteOffset = 12, .format = GraphicsFormat::RG32_FLOAT},
                         {.byteOffset = 20, .format = GraphicsFormat::RGB32_FLOAT}}}},
            .bindLayouts   = {m_GlobalBindLayout},
            .dynamicBuffer = m_UnifiedDynamicBufferHandle,
            .renderPass    = forwardPass.GetRenderPass().renderPass,
            .subpassIndex  = 0
    });
}

void SampleApp::CreateShadowmapPipeline()
{
    using namespace gore::gfx;

    RenderContext& renderContext = m_RenderSystem->GetRenderContext();
    AutoRenderPass shadowPass(&renderContext, renderPasses.shadowPassDesc);

    // Create a pipeline for the shadowmap rendering
    std::vector<char> vertexShaderBytecode   = sample::utils::LoadShaderBytecode("sample/Shadowmap", ShaderStage::Vertex, "main");
    std::vector<char> fragmentShaderBytecode = sample::utils::LoadShaderBytecode("sample/Shadowmap", ShaderStage::Fragment, "main");

    pipelines.shadowPipeline = renderContext.CreateGraphicsPipeline({
        GraphicsPipelineDesc{
            .debugName = "Shadowmap Pipeline",
            .VS{
                .byteCode  = reinterpret_cast<uint8_t*>(vertexShaderBytecode.data()),
                .byteSize  = static_cast<uint32_t>(vertexShaderBytecode.size()),
                .entryFunc = "vs"},
            .PS{
                .byteCode  = reinterpret_cast<uint8_t*>(fragmentShaderBytecode.data()),
                .byteSize  = static_cast<uint32_t>(fragmentShaderBytecode.size()),
                .entryFunc = "ps"},
            .colorFormats  = {},
            .depthFormat   = GraphicsFormat::D32_FLOAT,
            .stencilFormat = GraphicsFormat::Undefined,
            .vertexBufferBindings{
                {.byteStride = sizeof(Vector3) + sizeof(Vector2) + sizeof(Vector3),
                 .attributes =
                     {
                         {.byteOffset = 0, .format = GraphicsFormat::RGB32_FLOAT},
                         {.byteOffset = 12, .format = GraphicsFormat::RG32_FLOAT},
                         {.byteOffset = 20, .format = GraphicsFormat::RGB32_FLOAT}}}},
            .bindLayouts   = {m_GlobalBindLayout},
            .dynamicBuffer = m_UnifiedDynamicBufferHandle,
            .renderPass    = shadowPass.GetRenderPass().renderPass,
            .subpassIndex  = 0
        }
    });
}

void SampleApp::DrawTriangleWithRPSWrapper(const RpsCmdCallbackContext* pContext)
{
    RenderSystem& renderSystem = *reinterpret_cast<RenderSystem*>(pContext->pUserRecordContext);
    vk::CommandBuffer cmd      = rpsVKCommandBufferFromHandle(pContext->hCommandBuffer);

    DrawKey key = {"ForwardPass", AlphaMode::Opaque};

    renderSystem.DrawRenderer(key, cmd);
}

void SampleApp::ShadowmapPassWithRPSWrapper(const RpsCmdCallbackContext* pContext)
{
    // Update ShadowMap Descriptor Set
    VkImageView shadowmapView;
    RpsResult result = rpsVKGetCmdArgImageView(pContext, 0, &shadowmapView);

    RawBindGroupUpdateDesc updateDesc = {
        .textures = {{0, shadowmapView, BindType::SampledImage}},
    };

    RenderSystem& renderSystem = *reinterpret_cast<RenderSystem*>(pContext->pUserRecordContext);
    vk::CommandBuffer cmd      = rpsVKCommandBufferFromHandle(pContext->hCommandBuffer);

    DrawKey key = {"ShadowCaster", AlphaMode::Opaque};

    renderSystem.DrawRenderer(key, cmd);
}

void SampleApp::ForwardOpaquePassWithRPSWrapper(const RpsCmdCallbackContext* pContext)
{
    RenderSystem& renderSystem = *reinterpret_cast<RenderSystem*>(pContext->pUserRecordContext);
    vk::CommandBuffer cmd      = rpsVKCommandBufferFromHandle(pContext->hCommandBuffer);

    DrawKey key = {"ForwardPass", AlphaMode::Opaque};

    renderSystem.DrawRenderer(key, cmd);
}

void SampleApp::PrepareGraphics()
{
    CreateDefaultResources();
    CreateRenderPassDesc();
    CreateUnifiedGlobalDynamicBuffer();
    CreateGlobalBindGroup();
    CreatePipelines();
}

void SampleApp::CreateGlobalBindGroup()
{
    using namespace gore::gfx;

    RenderContext& renderContext = m_RenderSystem->GetRenderContext();
    m_GlobalConstantBuffer       = renderContext.CreateBuffer({.debugName = "Global Constant Buffer",
                                                               .byteSize  = sizeof(PerframeData),
                                                               .usage     = BufferUsage::Uniform,
                                                               .memUsage  = MemoryUsage::CPU_TO_GPU});
    
    m_ShadowmapSampler = renderContext.CreateSampler({
        .debugName = "Shadowmap Sampler",
    });

    std::vector<Binding> bindings{
        {0, BindType::UniformBuffer, 1, ShaderStage::Vertex},
        {1, BindType::SampledImage, 1, ShaderStage::Fragment},
        {2, BindType::Sampler, 1, ShaderStage::Fragment},
    };

    BindLayoutCreateInfo bindLayoutCreateInfo = {.name = "Global Descriptor Set Layout", .bindings = bindings};

    m_GlobalBindLayout = renderContext.GetOrCreateBindLayout(bindLayoutCreateInfo);

    m_GlobalBindGroup = renderContext.CreateBindGroup({
        .debugName       = "Global BindGroup",
        .updateFrequency = UpdateFrequency::PerFrame,
        .textures        = {{1, defaultResources.blackTexture}},
        .buffers         = {{0, m_GlobalConstantBuffer, 0, sizeof(PerframeData), BindType::UniformBuffer}},
        .samplers        = {{2, m_ShadowmapSampler}},
        .bindLayout      = &m_GlobalBindLayout,
    });
}

void SampleApp::Initialize()
{
    m_GraphicsCaps = m_RenderSystem->GetGraphicsCaps();

    InitializeRpsSystem();

    PrepareGraphics();

    Material forwardMat;
    forwardMat.SetAlphaMode(AlphaMode::Opaque);
    forwardMat.AddPass(Pass{
        .name      = "ShadowCaster",
        .shader    = pipelines.shadowPipeline,
        .bindGroup = {m_GlobalBindGroup}});
    
    forwardMat.AddPass(Pass{
        .name      = "ForwardPass",
        .shader    = pipelines.forwardPipeline,
        .bindGroup = {m_GlobalBindGroup}});
    
    gore::gfx::RenderContext& renderContext = m_RenderSystem->GetRenderContext();

    // GraphicsPipelineHandle forwardPipeline = renderContext.CreateGraphicsPipeline(
    //     GraphicsPipelineDesc{
    //         .debugName = "ForwardPipeline",
    //         .VS
    //     });

    // gore::Logger::Default().SetLevel(gore::LogLevel::DEBUG);

    scene = new gore::Scene("MainScene");

    gore::GameObject* cameraGameObject = scene->NewObject();
    cameraGameObject->SetName("MainCamera");
    gore::Camera* camera = cameraGameObject->AddComponent<gore::Camera>();
    cameraGameObject->AddComponent<CameraController>();

    gore::Transform* cameraTransform = cameraGameObject->GetComponent<gore::Transform>();
    cameraTransform->RotateAroundAxis(gore::Vector3::Right, gore::math::constants::PI_4);
    cameraTransform->SetLocalPosition(gore::Vector3::Backward * 20.0f + gore::Vector3::Up * 20.0f);

    // Light
    {
        gore::GameObject* lightGameObject = scene->NewObject();
        lightGameObject->SetName("Directional Light");

        gore::Light* light = lightGameObject->AddComponent<gore::Light>();
        light->SetType(gore::LightType::Directional);
    }

    {
        gore::GameObject* gameObject = scene->NewObject();
        gameObject->SetName("cube");
        gore::gfx::MeshRenderer* meshRenderer = gameObject->AddComponent<MeshRenderer>();
        meshRenderer->LoadMesh("cube.gltf");
        meshRenderer->SetMaterial(forwardMat);
        meshRenderer->SetDynamicBuffer(m_UnifiedDynamicBufferHandle);
        meshRenderer->SetDynamicBufferOffset(0);

        gore::Transform* transform = gameObject->GetTransform();
        transform->SetLocalPosition(gore::Vector3::Right * 20.0f);
    }

    {
        gore::GameObject* gameObject = scene->NewObject();
        gameObject->SetName("teapot");
        gore::gfx::MeshRenderer* meshRenderer = gameObject->AddComponent<MeshRenderer>();
        meshRenderer->LoadMesh("teapot.gltf");
    }

    {
        gore::GameObject* gameObject = scene->NewObject();
        gameObject->SetName("rock");

        gore::gfx::MeshRenderer* meshRenderer = gameObject->AddComponent<MeshRenderer>();
        meshRenderer->LoadMesh("rock.gltf");

        gore::Transform* transform = gameObject->GetTransform();
        transform->SetLocalPosition(gore::Vector3::Left * 10.0f);
    }
    // gore::GameObject* gameObject = scene->NewObject();
    // gameObject->SetName("TestObject O, T&R&S");

    // auto pSelfRotate           = gameObject->AddComponent<SelfRotate>();
    // auto pSelfScale            = gameObject->AddComponent<SelfScaleInBetweenRange>();
    // auto pSelfMoveBackAndForth = gameObject->AddComponent<SelfMoveBackAndForth>();

    // const float distance = 2.5f;

    // gameObject = scene->NewObject();
    // gameObject->SetName("TestObject L, T&S");
    // gameObject->GetTransform()->SetLocalPosition(gore::Vector3::Left * distance);
    // pSelfScale = gameObject->AddComponent<SelfScaleInBetweenRange>();
    // pSelfScale->SetMinMaxScale(0.5f, 1.5f);
    // pSelfMoveBackAndForth              = gameObject->AddComponent<SelfMoveBackAndForth>();
    // pSelfMoveBackAndForth->m_Direction = gore::Vector3::Left;
    // auto pLeftObject                   = gameObject;

    // gameObject = scene->NewObject();
    // gameObject->SetName("TestObject R, R&S");
    // gameObject->GetTransform()->SetLocalPosition(gore::Vector3::Right * distance);
    // pSelfRotate               = gameObject->AddComponent<SelfRotate>();
    // pSelfRotate->m_RotateAxis = gore::Vector3::Right;
    // pSelfScale                = gameObject->AddComponent<SelfScaleInBetweenRange>();
    // pSelfScale->SetMinMaxScale(0.5f, 1.5f);
    // auto pRightObject = gameObject;

    // gameObject = scene->NewObject();
    // gameObject->SetName("TestObject F, T&R");
    // gameObject->GetTransform()->SetLocalPosition(gore::Vector3::Forward * distance);
    // pSelfRotate                        = gameObject->AddComponent<SelfRotate>();
    // pSelfRotate->m_RotateAxis          = gore::Vector3::Forward;
    // pSelfMoveBackAndForth              = gameObject->AddComponent<SelfMoveBackAndForth>();
    // pSelfMoveBackAndForth->m_Direction = gore::Vector3::Forward;
    // auto pForwardObject                = gameObject;

    // gameObject = scene->NewObject();
    // gameObject->SetName("TestObject B, Translation only");
    // gameObject->GetTransform()->SetLocalPosition(gore::Vector3::Backward * distance);
    // pSelfMoveBackAndForth              = gameObject->AddComponent<SelfMoveBackAndForth>();
    // pSelfMoveBackAndForth->m_Direction = gore::Vector3::Backward;
    // auto pBackwardObject               = gameObject;

    // gameObject = scene->NewObject();
    // gameObject->SetName("TestObject U, Rotation only");
    // gameObject->GetTransform()->SetLocalPosition(gore::Vector3::Up * distance);
    // pSelfRotate               = gameObject->AddComponent<SelfRotate>();
    // pSelfRotate->m_RotateAxis = gore::Vector3::Up;
    // auto pUpObject            = gameObject;

    // gameObject = scene->NewObject();
    // gameObject->SetName("TestObject D, Scale only");
    // gameObject->GetTransform()->SetLocalPosition(gore::Vector3::Down * distance);
    // pSelfScale = gameObject->AddComponent<SelfScaleInBetweenRange>();
    // pSelfScale->SetMinMaxScale(0.5f, 1.5f);
    // auto pDownObject = gameObject;

    // pDownObject->GetTransform()->SetLocalScale(gore::Vector3::One * 1.2f);

    // gore::GameObject* childGameObject = scene->NewObject();
    // childGameObject->SetName("ChildObject");
    // childGameObject->GetTransform()->SetParent(pUpObject->GetTransform());
    // //    childGameObject->GetTransform()->SetParent(nullptr);
    // childGameObject->GetTransform()->SetLocalPosition(gore::Vector3::Up * 1.0f);
    // childGameObject->GetTransform()->SetLocalScale(gore::Vector3::One * 0.5f);

    // auto grandChildGameObject = scene->NewObject();
    // grandChildGameObject->SetName("GrandChildObject");
    // grandChildGameObject->GetTransform()->SetParent(childGameObject->GetTransform());
    // grandChildGameObject->GetTransform()->SetLocalPosition((gore::Vector3::Forward + gore::Vector3::Right) * 1.5f);
    // grandChildGameObject->GetTransform()->SetLocalScale(gore::Vector3::One * 0.7f);

    // auto pPeriodicallySwitchParent = grandChildGameObject->AddComponent<PeriodicallySwitchParent>();
    // pPeriodicallySwitchParent->SetParentAB(childGameObject->GetTransform(), pLeftObject->GetTransform());
    // pPeriodicallySwitchParent->m_RecalculateLocalPosition = true;

    // pPeriodicallySwitchParent = childGameObject->AddComponent<PeriodicallySwitchParent>();
    // pPeriodicallySwitchParent->SetParentAB(pUpObject->GetTransform(), childGameObject->GetTransform()->GetParent());
    // pPeriodicallySwitchParent->m_RecalculateLocalPosition = true;

    // auto pPeriodicallyChangeWorldTRS      = childGameObject->AddComponent<PeriodicallyChangeWorldTRS>();
    // pPeriodicallyChangeWorldTRS->m_Period = 0.5f;
    // pSelfScale                            = childGameObject->AddComponent<SelfScaleInBetweenRange>();
    // pSelfScale->SetMinMaxScale(0.5f, 1.5f);

    // childGameObject->AddComponent<SelfDestroyAfterSeconds>();

    // auto pLeftChildObject = scene->NewObject();
    // pLeftChildObject->SetName("LeftChildObject");
    // pLeftChildObject->GetTransform()->SetParent(pLeftObject->GetTransform());
    // pLeftChildObject->GetTransform()->SetLocalPosition(gore::Vector3::Left * 1.0f);
    // pLeftChildObject->GetTransform()->SetLocalScale(gore::Vector3::One * 0.5f);

    // auto pLeftGrandChildObject = scene->NewObject();
    // pLeftGrandChildObject->SetName("LeftGrandChildObject");
    // pLeftGrandChildObject->GetTransform()->SetParent(pLeftChildObject->GetTransform());
    // pLeftGrandChildObject->GetTransform()->SetLocalPosition((gore::Vector3::Forward + gore::Vector3::Left) * 1.5f);
    // pLeftGrandChildObject->GetTransform()->SetLocalScale(gore::Vector3::One * 0.7f);

    // std::vector<gore::GameObject*> destroyList = {pLeftObject, pRightObject, pLeftGrandChildObject};
    // auto pDeleteMultipleGameObjectsAfterSeconds =
    //     cameraGameObject->AddComponent<DeleteMultipleGameObjectsAfterSeconds>();
    // pDeleteMultipleGameObjectsAfterSeconds->SetGameObjectsToDelete(destroyList, 3, 2.0f);

    // LOG_STREAM(DEBUG) << "Find ChildObject in pUpObject at Initialization, non-recursively: "
    //                   << pUpObject->GetTransform()->Find("GrandChildObject")
    //                   << std::endl;
    // LOG_STREAM(DEBUG) << "Find ChildObject in pUpObject at Initialization, recursively: "
    //                   << pUpObject->GetTransform()->Find("GrandChildObject", true)
    //                   << std::endl;
    // LOG_STREAM(DEBUG) << "GetRoot() of grandChildGameObject: "
    //                   << grandChildGameObject->GetTransform()->GetRoot()->GetGameObject()->GetName()
    //                   << std::endl;
}

void SampleApp::Update()
{
    float deltaTime = GetDeltaTime();
    UpdateFPSText(deltaTime);

    Preupdate();
    UpdateImpl();
    PostUpdate();
    PreRender();
}

void SampleApp::Shutdown()
{
    delete scene;
}

void SampleApp::Preupdate()
{
}

void SampleApp::UpdateImpl()
{
}

void SampleApp::PostUpdate()
{
}

static int frameCount = 0;

void SampleApp::PreRender()
{
    Camera* mainCamera = Camera::Main;
    if (mainCamera == nullptr)
    {
        return;
    }

    PerframeData perframeData;
    perframeData.vpMatrix = mainCamera->GetViewProjectionMatrix();

    gore::gfx::RenderContext& renderContext = m_RenderSystem->GetRenderContext();
    renderContext.CopyDataToBuffer(m_GlobalConstantBuffer, perframeData);
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
