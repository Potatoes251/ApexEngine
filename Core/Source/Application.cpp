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
#include "Colliders.h"
#include "DebugRenderer.h"

#include "UI.h"
#include "Hud.h"

#include "Lighting/Lights.h"
#include "Lighting/LightManager.h"
#include "Lighting/ShadowManager.h"

#include "FileLog.h"
#include "LogSystem.h"
#include "Log.h"

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

Apex::Application::Application(IWindow* window, IGUI* gui)
{
    assert(m_instance == nullptr && "Only one Application instance allowed");
    m_instance = this;
    m_saveManager = std::make_unique<Save::SaveManager>("Saves/");

    InitLog();
    InitWindow(window);
    InitResourceManager();
    InitRHI();
    InitLighting();
    InitRendering();
    InitAudio();
    InitUI(gui);
    InitLua();
}

Apex::Application::~Application() = default;

void Apex::Application::InitResourceManager()
{
    m_resourceManager = std::make_unique<ResourceManager>();
}

void Apex::Application::InitLog()
{
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%d-%m-%Y-%H-%M-%S.log");

    m_fileLog = std::make_unique<FileLog>(oss.str());
    LogSystem::AddSink(m_fileLog.get());
}

void Apex::Application::InitWindow(IWindow* window)
{
    m_window = window;

    m_window->SetKeyCallback([&](Key key, bool down)
        {
            if (key == Key::Escape && down)
                m_window->Close();

            if (key == Key::O && down)
                if (m_onSaveRequested)
                    m_onSaveRequested();
        });
}

void Apex::Application::InitRHI()
{
    m_rhi = std::unique_ptr<IRHI>(CreateOpenGLRHI());

    m_resourceManager->SetRHI(m_rhi.get());
    m_rhi->SetClearColor(0.12f, 0.12f, 0.15f);
}

void Apex::Application::InitRendering()
{
    DebugRenderer::Initialize(m_resourceManager->CreateAsync<Shader>
        ("ApexAssets/Shaders/LineVert.glsl|ApexAssets/Shaders/LineFrag.glsl"), m_rhi.get());
}

void Apex::Application::InitAudio()
{
    m_audio = std::unique_ptr<IAudioEngine>(CreateAudioEngine());
}

void Apex::Application::InitUI(IGUI* gui)
{
    m_uiManager = std::make_unique<UIManager>(gui, m_resourceManager.get(), m_rhi.get());
}

void Apex::Application::InitLighting()
{
    m_lightManager = std::make_unique<LightManager>(m_rhi.get());
    m_shadowManager = std::make_unique<ShadowManager>(m_rhi.get(), 
        m_resourceManager->CreateAsync<Shader>("ApexAssets/Shaders/DirShadowVert.glsl|ApexAssets/Shaders/DirShadowFrag.glsl"), 
        m_resourceManager->CreateAsync<Shader>(
            "ApexAssets/Shaders/OmniShadowVert.glsl|ApexAssets/Shaders/OmniShadowFrag.glsl|ApexAssets/Shaders/OmniShadowGeo.glsl"));
}

void Apex::Application::InitLua()
{
    m_luaManager = std::make_unique<Scripting::LuaManager>();

    m_luaManager->RegisterHudBindings(m_uiManager.get());
    m_luaManager->RegisterScriptCast(this);
}

void Apex::Application::LoadScene(bool isBuild, bool startScene)
{
    std::string scenePath;

    if (isBuild)
    {
        scenePath = LoadBuildStartScene();          // build_scenes.json - first entry
        if (scenePath.empty())
            scenePath = "Assets/Scenes/scene.level"; // fallback
    }
    else
    {
        scenePath = LoadLastScene();               // Editor/prefs.ini
        if (scenePath.empty())
            scenePath = "Assets/Scenes/scene.level";

        EnsureDefaultScene(scenePath);
    }
    
    LoadScene(scenePath, startScene);
}

void Apex::Application::LoadScene(const std::string& path, bool startScene)
{
    // Reset scene
    m_scene = std::make_unique<Scene>();
    
    // Skybox
    auto cubemap = m_resourceManager->CreateAsync<Cubemap>(
        "ApexAssets/Textures/Skybox/px.png|ApexAssets/Textures/Skybox/nx.png|"
        "ApexAssets/Textures/Skybox/ny.png|ApexAssets/Textures/Skybox/py.png|"
        "ApexAssets/Textures/Skybox/pz.png|ApexAssets/Textures/Skybox/nz.png", true);

    auto cube = m_resourceManager->CreateAsync<Model>("ApexAssets/Meshes/cube/cube.mesh", true);
    auto shader = m_resourceManager->CreateAsync<Shader>(
        "ApexAssets/Shaders/SkyboxVert.glsl|ApexAssets/Shaders/SkyboxFrag.glsl", true);

    m_scene->SetSkybox(std::make_unique<Skybox>(cubemap, cube, shader));
    m_scene->SetShader(m_resourceManager->CreateAsync<Shader>("ApexAssets/Shaders/Mesh.vert|ApexAssets/Shaders/Mesh.frag", true));

    // Load scene file
    SceneLoader loader(m_resourceManager.get(), m_scene->GetPhysic(), m_luaManager.get());
    loader.LoadScene(*m_scene, path);
    
    m_resourceManager->WaitForAll();
    m_resourceManager->ProcessGpuUploads();
    
    // Rebuild physics
    for (auto& obj : m_scene->GetObjects())
    {
        auto* body = obj->GetComponent<RigidBodyComponent>();
        auto* collider = obj->GetComponent<Collider>();

        if (body && collider)
            m_scene->GetPhysic()->CreateActor(*collider, *body);
    }
    
    if (startScene)
        m_scene->Start();
}

void Apex::Application::Tick()
{
    if (!m_targetScene.empty())
    {
        LoadScene(m_targetScene, true);
        m_targetScene.clear();
    }

    m_deltaTime_s = m_window->GetDeltaTime();

    if (m_running)
    {
        constexpr float FIXED_UPDATE_TIME_S = 1.f / 60.f;

        m_scene->Update(m_deltaTime_s);
        m_accumulator_s += m_deltaTime_s;

        while (m_accumulator_s >= FIXED_UPDATE_TIME_S)
        {
            m_scene->FixedUpdate(FIXED_UPDATE_TIME_S);
            m_accumulator_s -= FIXED_UPDATE_TIME_S;
        }

        m_scene->LateUpdate(m_deltaTime_s);

        // listener is the camera so it updates after the camera
        UpdateAudioListener();
    }
}

void Apex::Application::UpdateAudioListener()
{
    LibMath::Vector3 pos = m_scene->GetMainCamera()->GetPosition();
    LibMath::Vector3 front = m_scene->GetMainCamera()->GetFront();
    LibMath::Vector3 up = m_scene->GetMainCamera()->GetUp();
    m_audio->SetListenerPosition(pos, front, up);
}

void Apex::Application::Render(UserInterface::IGUI* gui, RenderPassInfo pass)
{
    m_rhi->Clear();

    if (!pass.m_rhi) pass.m_rhi = m_rhi.get();

    m_scene->ComputePass(pass);

    if (pass.m_flags & PerformCompute)
        m_rhi->Barrier();

    VisibleLights visibleLights = m_scene->GetVisibleLights();

    m_lightManager->UploadToGpu(visibleLights);
    m_shadowManager->RenderShadows(visibleLights, m_scene.get(), pass.m_camera);
    m_shadowManager->BindShadowMaps(m_scene->GetShader());

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

    DebugRenderer::Get().DrawLines(pass.m_camera->GetViewProj());

    m_uiManager->RenderHUD(gui->GetBackgroundDrawList(), { 0, 0 }, { (float)m_window->GetWidth(), (float)m_window->GetHeight() });
}

void Apex::Application::EnsureDefaultScene(const std::string& path)
{
    if (std::filesystem::exists(path))
        return;

    // Create parent directories if needed
    std::filesystem::path p(path);
    if (p.has_parent_path())
        std::filesystem::create_directories(p.parent_path());

    std::ofstream file(path);
    if (!file.is_open())
    {
        LOG_ERROR("Failed to create default scene at: {}", path);
        return;
    }

    // Minimal valid .level file
    file << "{\n";
    file << "\"objects\": [\n";
    file << "]\n";
    file << "}";

    LOG_INFO("Created default scene at: {}", path);
}

std::string Apex::Application::LoadLastScene() const
{
    std::ifstream file(m_prefsPath);
    std::string line;
    while (std::getline(file, line))
    {
        if (line.rfind(m_lastSceneKey, 0) == 0)
            return line.substr(std::strlen(m_lastSceneKey));
    }
    return "";
}

std::string Apex::Application::LoadBuildStartScene() const
{
    std::ifstream file(m_buildScenesPath);
    if (!file.is_open())
        return "";

    std::string content(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>());

    // Expects: { "scenes": [ "Assets/Scenes/scene.level", ... ] }
    size_t scenePos = content.find("\"scenes\"");
    if (scenePos == std::string::npos)
        return "";

    size_t arrayOpen = content.find('[', scenePos);
    if (arrayOpen == std::string::npos)
        return "";

    size_t firstQuote = content.find('"', arrayOpen + 1);
    if (firstQuote == std::string::npos)
        return "";

    size_t secondQuote = content.find('"', firstQuote + 1);
    if (secondQuote == std::string::npos)
        return "";

    return content.substr(firstQuote + 1, secondQuote - firstQuote - 1);
}

void Apex::Application::AddCollectedObject(size_t id)
{
    if (std::find(
        m_collectedObjects.begin(),
        m_collectedObjects.end(),
        id) == m_collectedObjects.end())
    {
        m_collectedObjects.push_back(id);
    }
}

const std::vector<size_t>& Apex::Application::GetCollectedObjects() const
{
    return m_collectedObjects;
}

void Apex::Application::Pause()
{
    m_running = false;
    m_window->SetCursorCaptured(false);
}

void Apex::Application::Unpause()
{
    m_running = true;
    m_window->SetCursorCaptured(true);
}