#include "Application.h"

#include "LuaManager.h"
#include "Behaviour.h"
#include "LuaBehaviour.h"
#include "ScriptComponent.h"
#include "RenderPassInfo.h"
#include "SceneGraph.h"
#include "Object.h"

#include "SceneLoader.h"

#include "Window.h"
#include "RHI.h"
#include "Camera.h"
#include "Audio.h"


#include "ResourceManager.h"
#include "Mesh.h"
#include "Texture.h"
#include "Shader.h"
#include "Cubemap.h"
#include "Scene.h"
#include "BoxCollider.h"
#include "CapsuleCollider.h"
#include "MeshCollider.h"
#include "Rigidbody.h"

#include "DebugRenderer.h"
#include "UI.h"
#include "Hud.h"

#include <chrono>
#include <iostream>

#include <memory>
#include <filesystem>

using namespace Apex::Windowing;
using namespace Apex::Rendering;
using namespace Apex::Scripting;
using namespace Apex::Audio;
using namespace Apex::Resources;
using namespace Apex::UserInterface;
using namespace Apex::Physic;
using namespace Apex::Serialization;

namespace Apex
{
    Application::Application() = default;
    Application::~Application() = default;
}

void Apex::Application::Run()
{
    while (!m_window->ShouldClose())
    {   
        PollAllEvents();
        Tick();
    }

    Shutdown();
}

bool Apex::Application::Init(IWindow* window, IGUI* gui)
{
    if (!InitWindow(window)
    || !InitResourceManager()  
    || !InitCamera()
    || !InitRHI()  
    || !InitRendering()
    || !InitUI(gui)
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

bool Apex::Application::InitWindow(IWindow* window)
{
    m_window = window;

    if (!m_window)
        return false;

    m_window->SetKeyCallback([&](Key key, KeyAction action)
        {
            if (key == Key::Escape && action == KeyAction::Press)
                m_window->Close();

            if (key == Key::P && action == KeyAction::Press)
                m_scene->Save("../../../Assets/Scenes/scene.level");
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

bool Apex::Application::InitUI(IGUI* gui)
{
    if (!m_window) return false;

    m_uiManager = std::make_unique<UIManager>(gui);

    if (!m_uiManager)
        return false;

    m_uiManager->GetCanvas().AddElement(std::make_unique<UIText>("This is a test", Vector2(50.f, 50.f), 100.f, UIAnchor::Center, FontID::Default));

    return true;
}

void Apex::Application::LoadScene()
{
    Apex::Resources::ResourceHandle<Cubemap> cubemap = m_resourceManager->CreateAsync<Cubemap>(
        "Assets/Textures/Skybox/px.png|Assets/Textures/Skybox/nx.png|\
Assets/Textures/Skybox/ny.png|Assets/Textures/Skybox/py.png|\
Assets/Textures/Skybox/pz.png|Assets/Textures/Skybox/nz.png");
    Apex::Resources::ResourceHandle<Mesh> cube = m_resourceManager->CreateAsync<Mesh>("Assets/Meshes/cube.obj");
    Apex::Resources::ResourceHandle<Shader> shader = m_resourceManager->CreateAsync<Shader>("Assets/Shaders/SkyboxVert.glsl|Assets/Shaders/SkyboxFrag.glsl");

    auto sk = std::make_unique<Skybox>(cubemap, cube, shader, m_rhi.get());

    m_scene->SetSkybox(sk);


    SceneLoader loader(*m_resourceManager, *m_scene->GetPhysic());
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
            m_scene->GetPhysic()->CreateActor(*collider, *body);
    }
}

void Apex::Application::Shutdown()
{
    std::cout << "Application Shutdown\n" << std::endl;
}

void Apex::Application::Tick()
{
    //m_window->UpdateDeltaTime();

    constexpr float fixedUpdateTime_s = 1.f / 60.f;

    m_deltaTime_s = m_window->GetDeltaTime();
    if (m_deltaTime_s > 0.1f) m_deltaTime_s = fixedUpdateTime_s;

    ProcessInput();
    Update();

    m_accumulator_s += m_deltaTime_s;

    while (m_accumulator_s >= fixedUpdateTime_s)
    {
        m_scene->FixedUpdate(fixedUpdateTime_s);
        m_accumulator_s -= fixedUpdateTime_s;
    }
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
        m_deltaTime_s
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
    m_scene->Update(m_deltaTime_s);
}

void Apex::Application::Render(Camera* cam)
{
    if (!cam) cam = m_camera.get();
    m_rhi->Clear();

    RenderPassInfo pass;
    pass.m_aspectRatio = m_window->GetAspectRatio();
    pass.m_camera = cam;

    m_scene->Render(pass);

    LibMath::Matrix4 view = cam->GetViewMatrix();
    LibMath::Matrix4 proj = cam->GetProjectionMatrix(m_window->GetAspectRatio());

    DebugRenderer::Get().DrawLines(proj * view);
}