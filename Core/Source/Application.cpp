#include "Application.h"
#include "LuaBehaviour.h"
#include "ScriptComponent.h"

#include <chrono>
#include <iostream>

using namespace Apex::Windowing;
using namespace Apex::Rendering;
using namespace Apex::Scripting;
using namespace Apex::Audio;
using namespace Apex::Resources;
using namespace Apex::UserInterface;
using namespace Apex::Physic;

void Apex::Application::Run()
{
    while (!m_window->ShouldClose())
    {   
        PollAllEvents();
        Tick();
    }

    Shutdown();
}

bool Apex::Application::Init(Apex::Windowing::IWindow* shareContext)
{
    std::filesystem::current_path(std::filesystem::path(m_argv0).parent_path());

    std::cout << "Application Init\n";

    if (!InitResourceManager())     return false;
    if (!InitCamera())              return false;
    if (!InitWindow(shareContext))  return false;
    if (!InitRHI())                 return false;
    if (!InitRendering())           return false;
    if (!InitAudio())               return false;
    if (!InitScene())               return false;

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

bool Apex::Application::InitWindow(Apex::Windowing::IWindow* shareContext)
{
    m_window = std::unique_ptr<IWindow>(CreateWindow("Apex Scene", 1280, 720, shareContext));

    if (!m_window)
        return false;

    m_window->SetCursorCaptured(true);

    m_window->SetKeyCallback([&](Key key, bool pressed)
        {
            if (key == Key::Escape && pressed)
                m_window->Close();
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
    m_uiManager = std::make_unique<UIManager>(m_window->NativeHandle());

    if (!m_uiManager)
        return false;

    m_uiManager->GetCanvas().AddElement(std::make_unique<UIText>("This is a test", Vector2(50.f, 50.f), 100, FontID::Default));

    return true;
}

void Apex::Application::LoadScene()
{
    Apex::Resources::ResourceHandle<Shader> m_shader = m_resourceManager->CreateAsync<Shader>("Assets/Shaders/Cube.vert|Assets/Shaders/Cube.frag");
    Apex::Resources::ResourceHandle<Mesh> m_sphere = m_resourceManager->CreateAsync<Mesh>("Assets/Meshes/sphere.obj");
    Apex::Resources::ResourceHandle<Mesh> m_cube = m_resourceManager->CreateAsync<Mesh>("Assets/Meshes/cube.obj");
    Apex::Resources::ResourceHandle<Texture> m_texture = m_resourceManager->CreateAsync<Texture>("Assets/Textures/container.jpg");
    Apex::Resources::ResourceHandle<Texture> m_texture2 = m_resourceManager->CreateAsync<Texture>("Assets/Textures/T_M_B_44_Qishilong_body01_B.png");

    auto obj = m_scene->CreateObject();

    auto& renderer = obj->AddComponent<MeshRenderer>();
    renderer.SetMesh(m_sphere);
    renderer.SetTexture(m_texture);
    renderer.SetShader(m_shader);

    auto& body = obj->AddComponent<RigidBodyComponent>(1, BodyType::Dynamic);
    body.SetPosition({ 0,10,0 });

    auto obj2 = m_scene->CreateObject();

    auto& renderer2 = obj2->AddComponent<MeshRenderer>();
    renderer2.SetMesh(m_cube);
    renderer2.SetTexture(m_texture2);
    renderer2.SetShader(m_shader);

    auto& body2 = obj2->AddComponent<RigidBodyComponent>(0, BodyType::Static);
    body2.SetPosition({ 0,0,0 });

    auto& collider = obj->AddComponent<MeshCollider>(m_sphere, MeshColliderType::Convex, false);
    auto& collider2 = obj2->AddComponent<BoxCollider>(LibMath::Vector3{ 10.f, .5f, 10.f }, false);

    while (!m_sphere.IsReady() ||
        !m_cube.IsReady() ||
        !m_texture.IsReady() ||
        !m_texture2.IsReady() ||
        !m_shader.IsReady())
        m_resourceManager->ProcessGpuUploads();

    m_physics.CreateActor(collider, body);
    m_physics.CreateActor(collider2, body2);
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
    BoxCollider box({ 1,1,1 }, false);
    std::vector<HitResult> hit;
    if (m_physics.Sweep(hit, m_camera->GetPosition(), m_camera->GetFront(), box, 1000.f)) printf("hit at %f size : %zd\r", hit[0].m_distance, hit.size());
    else printf("no hit\r");

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
    m_rhi->Clear(0.12f, 0.12f, 0.15f);

    LibMath::Matrix4 view = m_camera->GetViewMatrix();
    LibMath::Matrix4 proj = m_camera->GetProjectionMatrix(m_window->GetAspectRatio());

    m_scene->Render(view, proj, m_camera->GetPosition());

    m_physics.DrawColliders();
    DebugRenderer::Get().DrawLines(proj * view);

    m_resourceManager->ProcessGpuUploads();
}