#include "Application.h"
#include "LuaBehaviour.h"
#include "ScriptComponent.h"
#include "RenderPassInfo.h"

#include "SceneLoader.h"

#include <chrono>
#include <iostream>

#include <filesystem>

using namespace Apex::Windowing;
using namespace Apex::Rendering;
using namespace Apex::Scripting;
using namespace Apex::Audio;
using namespace Apex::Resources;
using namespace Apex::UserInterface;
using namespace Apex::Physic;
using namespace Apex::Serialization;

void Apex::Application::Run()
{
    while (!m_window->ShouldClose())
    {   
        PollAllEvents();
        Tick();
    }

    Shutdown();
}

bool Apex::Application::Init(bool createWindow)
{
    std::cout << "Application Init\n";

    if (createWindow && !InitWindow()) return false;

    if (!InitResourceManager()  
    || !InitCamera()
    || !InitRHI()  
    || !InitRendering()
    || !InitAudio()
    || !InitScene())

        return false;

    return true;
}

bool Apex::Application::InitResourceManager()
{
    m_resourceManager = std::make_unique<ResourceManager>(m_threadPool);
    return true;
}

bool Apex::Application::InitCamera()
{
    m_camera = std::make_unique<Camera>();
    return true;
}

bool Apex::Application::InitWindow()
{
    m_window = std::unique_ptr<IWindow>(CreateWindow("Apex Scene", 1280, 720, nullptr));

    if (!m_window)
        return false;

    m_window->SetCursorCaptured(true);

    m_window->SetKeyCallback([&](Key key, KeyAction action)
        {
            if (key == Key::Escape && action == KeyAction::Press)
                m_window->Close();

            if (key == Key::P && action == KeyAction::Press)
                m_scene->Save("Assets/Scenes/scene.level", m_physics);
        });

    m_window->SetMouseMoveCallback([&](double dx, double dy)
        {
            m_camera->ProcessMouse(dx, dy);
        });

    m_window->SetScrollCallback([&](double, double yoff)
        {
            m_camera->ProcessScroll(yoff);
        });

    return true;
}

bool Apex::Application::InitRHI()
{
    m_rhi = std::unique_ptr<IRHI>(CreateOpenGLRHI());

    if (!m_rhi)
        return false;

    m_resourceManager->SetRHI(m_rhi.get());
    m_rhi->SetClearColor(0.12f, 0.12f, 0.15f);

    return true;
}

bool Apex::Application::InitRendering()
{
    DebugRenderer::Initialize(m_resourceManager->CreateAsync<Shader>("Assets/Shaders/LineVert.glsl|Assets/Shaders/LineFrag.glsl"), m_rhi.get());

    return true;
}

bool Apex::Application::InitAudio()
{
    m_audio = std::unique_ptr<IAudioEngine>(CreateAudioEngine());

    if (m_audio)
    {
        m_audio->Preload("fragments of hope.ogg");

        //m_audio->Play2D("fragments of hope.ogg",
        //    { .m_loop = true, .m_volume = 1.f });
    }

    return true;
}

bool Apex::Application::InitScene()
{
    m_scene = std::make_unique<Apex::Rendering::Scene>();
    return true;
}

bool Apex::Application::InitUI()
{
    if (!m_window) return false;

    m_uiManager = std::make_unique<UIManager>(m_window->NativeHandle());

    if (!m_uiManager)
        return false;

    m_uiManager->GetCanvas().AddElement(std::make_unique<UIText>("This is a test", Vector2(50.f, 50.f), 100.f, UIAnchor::Center, FontID::Default));

    return true;
}

void Apex::Application::LoadScene()
{
    SceneLoader loader(*m_resourceManager.get(), m_physics);
    loader.LoadScene(*m_scene, "Assets/Scenes/scene.level");

    bool ready = false;

    while (!ready)
    {
        ready = true;

        for (auto& objPtr : m_scene->GetObjects())
        {
            Object* obj = objPtr.get();

            if (auto* renderer = obj->GetComponent<MeshRenderer>())
            {
                if (!renderer->GetMesh().IsReady())
                    ready = false;

                if (!renderer->GetTexture().IsReady())
                    ready = false;

                if (!renderer->GetShader().IsReady())
                    ready = false;
            }

            if (auto* collider = obj->GetComponent<MeshCollider>())
            {
                if (!collider->GetMesh().IsReady())
                    ready = false;
            }
        }

        m_resourceManager->ProcessGpuUploads();
    }

    for (auto& objPtr : m_scene->GetObjects())
    {
        Object* obj = objPtr.get();

        auto* body = obj->GetComponent<RigidBodyComponent>();
        auto* collider = obj->GetComponent<Collider>();

        if (body && collider)
            m_physics.CreateActor(*collider, *body);
    }
}

void Apex::Application::Shutdown()
{
    std::cout << "Application Shutdown\n" << std::endl;
}

void Apex::Application::Tick()
{
    m_window->MakeContextCurrent();
    m_window->UpdateDeltaTime();

    m_deltaTime = m_window->GetDeltaTime();
    if (m_deltaTime > 0.1f) m_deltaTime = 1.f / 60.f;

    ProcessInput();
    Update();

    m_accumulator += m_deltaTime;
    m_totalTime += m_deltaTime;

    while (m_accumulator >= 1.f / 60.f)
    {
        m_physics.Update(1.f / 60.f);
        m_accumulator -= 1.f / 60.f;
    }

    SyncPhysicsToTransforms();

    Render();

    m_window->SwapBuffers();
}

void Apex::Application::ProcessInput()
{
    m_camera->ProcessKeyboard(
        m_window->IsKeyPressed(Key::W),
        m_window->IsKeyPressed(Key::S),
        m_window->IsKeyPressed(Key::A),
        m_window->IsKeyPressed(Key::D),
        m_window->IsKeyPressed(Key::E),
        m_window->IsKeyPressed(Key::Q),
        m_deltaTime
    );
}

void Apex::Application::UpdateAudioListener()
{
    if (m_audio)
    {
        LibMath::Vector3 pos = m_camera->GetPosition();
        LibMath::Vector3 front = m_camera->GetFront();
        m_audio->SetListenerPosition(
            { pos[0],   pos[1],   pos[2] },
            { front[0], front[1], front[2] }
        );
    }
}

void Apex::Application::Update()
{
    m_scene->Update(m_deltaTime);
}

void Apex::Application::SyncPhysicsToTransforms()
{
    m_physics.SyncFromPhysx();
}

void Apex::Application::Render()
{
    m_rhi->Clear();

    RenderPassInfo pass;
    pass.m_aspectRatio = m_window->GetAspectRatio();
    pass.m_camera = m_camera.get();

    m_scene->Render(pass);

    LibMath::Matrix4 view = m_camera->GetViewMatrix();
    LibMath::Matrix4 proj = m_camera->GetProjectionMatrix(m_window->GetAspectRatio());

    m_physics.DrawColliders();
    DebugRenderer::Get().DrawLines(proj * view);

    m_resourceManager->ProcessGpuUploads();
}