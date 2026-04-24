#include "Application.h"


#include "LuaManager.h"
#include "SceneLoader.h"
#include "ScriptComponent.h"

#include "Audio.h"
#include "Window.h"

#include "RHI.h"
#include "Camera.h"
#include "SceneGraph.h"
#include "RenderPassInfo.h"

#include "Mesh.h"
#include "Object.h"
#include "Shader.h"
#include "Cubemap.h"
#include "Texture.h"
#include "ResourceManager.h"

#include "Rigidbody.h"
#include "BoxCollider.h"
#include "MeshCollider.h"
#include "DebugRenderer.h"
#include "CapsuleCollider.h"

#include "UI.h"
#include "Hud.h"

#include "Lighting/Lights.h"
#include "Lighting/LightManager.h"
#include "Lighting/ShadowManager.h"

#include "FileLog.h"
#include "LogSystem.h"

#include <chrono>
#include <iostream>

#include <memory>
#include <filesystem>

using namespace Apex::Data;
using namespace Apex::Audio;
using namespace Apex::Input;
using namespace Apex::Physic;
using namespace Apex::Lighting;
using namespace Apex::Windowing;
using namespace Apex::Rendering;
using namespace Apex::Scripting;
using namespace Apex::Resources;
using namespace Apex::UserInterface;
using namespace Apex::Serialization;

Apex::Application::Application() = default;
Apex::Application::~Application() = default;

bool Apex::Application::Init(IWindow* window)
{
    if (!InitLog()
    || !InitWindow(window)
    || !InitResourceManager()  
    || !InitRHI()  
    || !InitLighting()
    || !InitRendering()
    || !InitAudio()
    || !InitLua()
    || !InitScene())
        return false;

    return true;
}

bool Apex::Application::InitGUI(UserInterface::IGUI* gui)
{
    return InitUI(gui);
}

bool Apex::Application::InitResourceManager()
{
    m_resourceManager = std::make_unique<ResourceManager>();
    return true;
}

bool Apex::Application::InitLog()
{
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%d-%m-%Y-%H-%M-%S.log");

    m_fileLog = std::make_unique<FileLog>(oss.str());
    LogSystem::AddSink(m_fileLog.get());

    return true;
}

bool Apex::Application::InitWindow(IWindow* window)
{
    m_window = window;

    if (!m_window)
        return false;

    m_window->SetKeyCallback([&](Key key, bool down)
        {
            if (key == Key::Escape && down)
                m_window->Close();
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
    DebugRenderer::Initialize(m_resourceManager->CreateAsync<Shader>
        ("ApexAssets/Shaders/LineVert.glsl|ApexAssets/Shaders/LineFrag.glsl"), m_rhi.get());

    return true;
}

bool Apex::Application::InitAudio()
{
    m_audio = std::unique_ptr<IAudioEngine>(CreateAudioEngine());

    if (m_audio)
        m_audio->Preload("fragments of hope.ogg");

    return true;
}

bool Apex::Application::InitUI(IGUI* gui)
{
    if (!m_window) return false;

    m_uiManager = std::make_unique<UIManager>(gui, m_resourceManager.get(), m_rhi.get());

    if (!m_uiManager)
        return false;

    return true;
}

bool Apex::Application::InitLighting()
{
    m_lightManager = std::make_unique<LightManager>(m_rhi.get());
    m_shadowManager = std::make_unique<ShadowManager>(m_rhi.get(), 
        m_resourceManager->CreateAsync<Shader>("ApexAssets/Shaders/DirShadowVert.glsl|ApexAssets/Shaders/DirShadowFrag.glsl"), 
        m_resourceManager->CreateAsync<Shader>(
            "ApexAssets/Shaders/OmniShadowVert.glsl|ApexAssets/Shaders/OmniShadowFrag.glsl|ApexAssets/Shaders/OmniShadowGeo.glsl"));
    return true;
}

bool Apex::Application::InitLua()
{
    m_luaManager = std::make_unique<Scripting::LuaManager>();
    if (!m_luaManager) return false;

    if (m_uiManager)
        m_luaManager->RegisterHudBindings(m_uiManager.get());

    return true;
}

bool Apex::Application::InitScene()
{
    m_scene = std::make_unique<Apex::Rendering::Scene>();
    return true;
}

void Apex::Application::LoadScene()
{
    Apex::Resources::ResourceHandle<Cubemap> cubemap = m_resourceManager->CreateAsync<Cubemap>(
        "ApexAssets/Textures/Skybox/px.png|ApexAssets/Textures/Skybox/nx.png|"
        "ApexAssets/Textures/Skybox/ny.png|ApexAssets/Textures/Skybox/py.png|"
        "ApexAssets/Textures/Skybox/pz.png|ApexAssets/Textures/Skybox/nz.png", true);
    Apex::Resources::ResourceHandle<Mesh> cube = m_resourceManager->CreateAsync<Mesh>("ApexAssets/Meshes/cube/cube.mesh", true);
    Apex::Resources::ResourceHandle<Shader> shader = m_resourceManager->CreateAsync<Shader>("ApexAssets/Shaders/SkyboxVert.glsl|ApexAssets/Shaders/SkyboxFrag.glsl", true);

    auto sk = std::make_unique<Skybox>(cubemap, cube, shader);

    m_scene->SetSkybox(sk);
    
    SceneLoader loader(m_resourceManager.get(), m_scene->GetPhysic(), m_luaManager.get());
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

                if (!renderer->GetMaterial().IsReady())
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

    for (auto& obj : m_scene->GetObjects())
    {
        auto* body = obj->GetComponent<RigidBodyComponent>();
        auto* collider = obj->GetComponent<Collider>();

        if (body && collider)
            m_scene->GetPhysic()->CreateActor(*collider, *body);
    }
}

void Apex::Application::Tick()
{
    constexpr float fixedUpdateTime_s = 1.f / 60.f;

    m_deltaTime_s = m_window->GetDeltaTime();
    if (m_deltaTime_s > 0.1f) m_deltaTime_s = fixedUpdateTime_s;

    m_scene->Update(m_deltaTime_s);

    m_accumulator_s += m_deltaTime_s;

    while (m_accumulator_s >= fixedUpdateTime_s)
    {
        m_scene->FixedUpdate(fixedUpdateTime_s);
        m_accumulator_s -= fixedUpdateTime_s;
    }

    m_scene->LateUpdate(m_deltaTime_s);
}

void Apex::Application::UpdateAudioListener()
{
    if (m_audio)
    {
        LibMath::Vector3 pos = m_scene->GetMainCamera()->GetPosition();
        LibMath::Vector3 front = m_scene->GetMainCamera()->GetFront();
        m_audio->SetListenerPosition(
            { pos[0],   pos[1],   pos[2] },
            { front[0], front[1], front[2] }
        );
    }
}

void Apex::Application::Render(RenderPassInfo pass)
{
    m_rhi->Clear();

    VisibleLights visibleLights = m_scene->GetVisibleLights();

    m_lightManager->UploadToGpu(visibleLights);
    //m_shadowManager->RenderShadows(visibleLights, m_scene.get(), pass.m_camera);
    //m_shadowManager->BindShadowMaps();

    if (pass.m_target != RHI_INVALID)
    {
        m_rhi->BindFrameBuffer(pass.m_target, pass.m_width, pass.m_height);
        m_rhi->Clear();
    }
    else
    {
        m_rhi->UnBindFrameBuffer(m_window->GetWidth(), m_window->GetHeight());
        m_rhi->Clear();
    }

    m_scene->Render(pass);

    //if (pass.m_flags & ShowCollider)
        DebugRenderer::Get().DrawLines(pass.m_camera->GetViewProj());
}