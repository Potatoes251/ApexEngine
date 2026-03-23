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
        Tick();
    }

    Shutdown();
}

bool Apex::Application::Init()
{
    std::filesystem::current_path(std::filesystem::path(m_argv0).parent_path());

    std::cout << "Application Init\n";

    if (!InitResourceManager()) return false;
    if (!InitCamera())          return false;
    if (!InitWindow())          return false;
    if (!InitRHI())             return false;
    if (!InitRendering())       return false;
    if (!InitAudio())           return false;
    if (!InitScene())           return false;
    if (!InitAnimation())       return false;

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
    m_window = std::unique_ptr<IWindow>(CreateWindow("Apex Renderer", 1280, 720));

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
    DebugRenderer::Initialize(m_resourceManager->CreateAsync<Shader>("Assets/Shaders/LineVert.glsl|Assets/Shaders/LineFrag.glsl"));

    return true;
}

bool Apex::Application::InitAudio()
{
    m_audio = std::unique_ptr<IAudioEngine>(CreateAudioEngine());

    if (m_audio)
    {
        m_audio->Preload("fragments of hope.ogg");

        m_audio->Play2D("fragments of hope.ogg",
            { .m_loop = true, .m_volume = 1.f });
    }

    return true;
}

bool Apex::Application::InitScene()
{
    m_scene = std::make_unique<Apex::Rendering::Scene>(m_rhi.get(), m_threadPool);
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

bool Apex::Application::InitAnimation()
{
    m_animationMesh = m_resourceManager->CreateAsync<Mesh>("Assets/Meshes/Withered_Freddy.fbx");
    m_animationShader = m_resourceManager->CreateAsync<Shader>("Assets/Shaders/SkinningVert.glsl|Assets/Shaders/SkinningFrag.glsl");
    //m_animationShader = m_resourceManager->CreateAsync<Shader>("Assets/Shaders/Cube.vert|Assets/Shaders/Cube.frag");

    m_animator = std::make_shared<Apex::Rendering::Animator>(m_animationMesh, m_animationShader, 30.f);

    m_animator->Play(16);
    
    return true;
}

void Apex::Application::LoadScene()
{
    m_sphere = m_resourceManager->CreateAsync<Mesh>("Assets/Meshes/sphere.obj");
    m_texture = m_resourceManager->CreateAsync<Texture>("Assets/Textures/container.jpg");
    m_texture2 = m_resourceManager->CreateAsync<Texture>("Assets/Textures/T_M_B_44_Qishilong_body01_B.png");
    m_shader = m_resourceManager->CreateAsync<Shader>("Assets/Shaders/Cube.vert|Assets/Shaders/Cube.frag");

    m_cube = m_resourceManager->CreateAsync<Mesh>("Assets/Meshes/cube.obj");

    while (!m_sphere.IsReady() ||
        !m_cube.IsReady() ||
        !m_texture.IsReady() ||
        !m_texture2.IsReady() ||
        !m_shader.IsReady())
        m_resourceManager->ProcessGpuUploads();

    BoxCollider collider = BoxCollider({ 1.f, 1.f, 1.f });
    BoxCollider collider2 = BoxCollider({ 10.f, .5f, 10.f });

    auto obj = std::make_unique<Object>();
    auto& body = obj->AddComponent<RigidBodyComponent>(1, BodyType::Dynamic);
    body.SetPosition({0, 10, 0});
    m_body = &body;
    m_objects.push_back(std::move(obj));

    auto obj2 = std::make_unique<Object>();
    auto& body2 = obj2->AddComponent<RigidBodyComponent>(0, BodyType::Static);
    body2.SetPosition({0, 0, 0});
    m_body2 = &body2;
    m_objects.push_back(std::move(obj2));

    MeshCollider meshCollider = MeshCollider(m_sphere, MeshColliderType::Convex);
    //MeshCollider meshCollider2 = MeshCollider(mesh2, MeshColliderType::Triangle);

    CapsuleCollider capsule(2, 2);

    m_physics.CreateActor(meshCollider, body);
    m_physics.CreateActor(collider2, body2);
}

void Apex::Application::Shutdown()
{
    std::cout << "Application Shutdown\n" << std::endl;
}

void Apex::Application::Tick()
{
    m_window->PollEvents();

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

    m_resourceManager->ProcessGpuUploads();
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
    for (auto& obj : m_objects)
        obj->Update(m_deltaTime);
}

void Apex::Application::SyncPhysicsToTransforms()
{
    m_physics.SyncFromPhysx();
}

void Apex::Application::Render()
{
    m_rhi->Clear(0.12f, 0.12f, 0.15f);

    m_shader->Use();
    m_resourceManager->Get<Texture>("Assets/Textures/container.jpg")->Bind();

    LibMath::Matrix4 view = m_camera->GetViewMatrix();
    LibMath::Matrix4 proj = m_camera->GetProjectionMatrix(m_window->GetAspectRatio());

    m_shader->SetUniform("uModel", static_cast<LibMath::Matrix4>(m_body->GetTransform()));
    m_shader->SetUniform("uView", view);
    m_shader->SetUniform("uProjection", proj);
    m_shader->SetUniform("uCameraPos", m_camera->GetPosition());

    m_shader->SetUniform("uLightPos", LibMath::Vector3(2.f, 3.f, 2.f));
    m_shader->SetUniform("uLightColor", LibMath::Vector3(1.f, 1.f, 1.f));
    m_shader->SetUniform("uViewPos", m_camera->GetPosition());

    m_shader->SetUniform("uTexture", 0);
    m_shader->SetUniform("uUseTexture", 1);

    m_sphere->Draw();

    m_texture2->Bind();
    m_shader->SetUniform("uModel", static_cast<LibMath::Matrix4>(m_body2->GetTransform()));
    //m_cube->Draw();

    //animation

    m_animationShader->Use();

    m_animationShader->SetUniform("uModel", LibMath::Matrix4::identity());
    m_animationShader->SetUniform("uView", view);
    m_animationShader->SetUniform("uProjection", proj);
    //m_animationShader->SetUniform("uCameraPos", m_camera->GetPosition());

    m_animator->Update(m_deltaTime);

    m_animationMesh->Draw();

    m_physics.DrawColliders();
    DebugRenderer::Get().DrawLines(m_rhi.get(), proj * view);

    m_resourceManager->ProcessGpuUploads();
}