#include "Editor.h"
#include "EditorIcon.h"

#include "Shader.h"
#include "ResourceHandle.h"
#include "Audio.h"
#include "ResourceManager.h"
#include "Scene.h"
#include "SceneLoader.h"

#include "Rigidbody.h"


using namespace Apex::Windowing;
using namespace Apex::UserInterface;
using namespace Apex::Audio;
using namespace Apex::Rendering;
using namespace Apex::Serialization;
using namespace Apex::Physic;

namespace Apex::Editor 
{
    bool Editor::InitWindow()
    {
        m_window = std::unique_ptr<IWindow>(CreateWindow("Apex Editor", 1280, 720));

        if (!m_window) return false;

        m_window->Maximize();
        return true;
    }

    bool Editor::InitPanels(Apex::Application& app)
    {
        m_app = &app;
        m_audio = app.GetAudio();
        m_gui = std::unique_ptr<IGUI>(CreateGUI(m_window->NativeHandle()));

        if (!m_gui) return false;

        // Create editor scene
        m_editScene = std::make_unique<Apex::Rendering::Scene>();

        Apex::Resources::ResourceHandle<Cubemap> cubemap = m_app->GetResourceManager()->CreateAsync<Cubemap>(
            "ApexAssets/Textures/Skybox/px.png|ApexAssets/Textures/Skybox/nx.png|\
ApexAssets/Textures/Skybox/ny.png|ApexAssets/Textures/Skybox/py.png|\
ApexAssets/Textures/Skybox/pz.png|ApexAssets/Textures/Skybox/nz.png");
        Apex::Resources::ResourceHandle<Mesh> cube = m_app->GetResourceManager()->CreateAsync<Mesh>("Assets/Meshes/cube.obj");
        Apex::Resources::ResourceHandle<Shader> shader = m_app->GetResourceManager()->CreateAsync<Shader>("ApexAssets/Shaders/SkyboxVert.glsl|ApexAssets/Shaders/SkyboxFrag.glsl");

        auto sk = std::make_unique<Skybox>(cubemap, cube, shader, m_app->GetRhi());

        m_editScene->SetSkybox(sk);

        // Load scene
        SceneLoader loader(*app.GetResourceManager(), *m_editScene->GetPhysic());
        loader.LoadScene(*m_editScene, "Assets/Scenes/scene.level");

        bool ready = false;

        while (!ready)
        {
            ready = true;

            for (auto& objPtr : m_editScene->GetObjects())
            {
                Object* obj = objPtr.get();

                if (auto* renderer = obj->GetComponent<MeshRenderer>())
                {
                    auto& mesh = renderer->GetMesh();

                    if (!mesh.IsReady())
                        ready = false;
                }

                if (auto* collider = obj->GetComponent<MeshCollider>())
                {
                    auto mesh = collider->GetMesh();

                    if (!mesh.IsReady())
                        ready = false;
                }
            }

            m_app->GetResourceManager()->ProcessGpuUploads();
        }

        // Init physics
        for (auto& objPtr : m_editScene->GetObjects())
        {
            Object* obj = objPtr.get();

            auto* body = obj->GetComponent<RigidBodyComponent>();
            auto* collider = obj->GetComponent<Collider>();

            if (body && collider)
                m_editScene->GetPhysic()->CreateActor(*collider, *body);
        }

        // Give scene to Application
        m_app->SetScene(m_editScene.get());

        // All panels use m_editScene
        m_componentsViewer = std::make_unique<ComponentsViewer>(
            *m_gui,
            *m_editScene,
            *m_app->GetResourceManager(),
            *m_editScene->GetPhysic(),
            *m_app->GetLuaManager()
        );

        m_hierarchy = std::make_unique<Hierarchy>(
            *m_gui,
            m_editScene.get(),
            m_componentsViewer.get()
        );

        m_viewport = std::make_unique<Viewport>(
            app.GetResourceManager()->CreateAsync<Shader>("ApexAssets/Shaders/GizmoVert.glsl|ApexAssets/Shaders/GizmoFrag.glsl"),
            app.GetResourceManager()->CreateAsync<Shader>("ApexAssets/Shaders/GridVert.glsl|ApexAssets/Shaders/GridFrag.glsl"),
            m_editScene.get(),
            app.GetRhi(),
            m_gui.get(),
            this
        );

        m_contentBrowser = std::make_unique<ContentBrowser>(*m_gui, *m_window, "Assets/" ,*app.GetRhi(), *app.GetResourceManager());

        m_viewport = std::make_unique<Viewport>(
            app.GetResourceManager()->CreateAsync<Shader>("ApexAssets/Shaders/GizmoVert.glsl|ApexAssets/Shaders/GizmoFrag.glsl"),
            app.GetResourceManager()->CreateAsync<Shader>("ApexAssets/Shaders/GridVert.glsl|ApexAssets/Shaders/GridFrag.glsl"),
            app.GetScene(), app.GetRhi(), m_gui.get(), this);

        m_window->SetKeyCallback([&](Key key, KeyAction action)
            {
                if (key == Key::P && action == KeyAction::Press)
                    m_app->GetScene()->Save("Assets/Scenes/scene.level");

                if (key == Key::F && action == KeyAction::Press)
                    m_viewport->ToggleFullScreen();
            });

        m_window->SetMouseMoveCallback([&](double mouseX, double mouseY)
            {
                if (m_window->IsMousButtonPressed(Windowing::MouseButton::Right))
                    m_viewport->GetCamera().ProcessMouse(mouseX, mouseY);
                else
                {
                    Windowing::MousePosition pos = m_window->GetMousePosition();
                    m_viewport->OnMouseMove(pos.m_x, pos.m_y);
                }
            });

        m_window->SetMouseButtonCallback([&](Windowing::MouseButton button, bool press)
            {
                if (button == Windowing::MouseButton::Left && press)
                {
                    Windowing::MousePosition pos = m_window->GetMousePosition();
                    m_viewport->OnMouseClick(pos.m_x, pos.m_y);
                }
                if (button == Windowing::MouseButton::Left && !press)
                {
                    Windowing::MousePosition pos = m_window->GetMousePosition();
                    m_viewport->OnMouseRelease(pos.m_x, pos.m_y);
                }
            });

        m_window->MakeContextCurrent();

        m_contentBrowser->SetOnAssetLoad([this](const AssetEntry& entry)
            {
                switch (entry.m_type)
                {
                case AssetType::Texture:
                    OpenTextureEditor(entry.m_path);
                    break;
                case AssetType::Mesh:
                    OpenMeshViewer(entry.m_path);
                    break;
                case AssetType::Sound:
                    OpenSoundPlayer(entry.m_path);
                    break;
                default:
                    break;
                }
            });

        return true;
    }

    void Editor::OnGameStopped()
    {
        m_app = nullptr;
    }

    void Editor::Render()
    {
        if (!m_window) return;

        m_window->Clear();

        if (!m_gui) return;

        if (m_contentBrowser)
            m_contentBrowser->PrepareThumbnails();

        m_gui->BeginFrame();

        m_gui->StartDockSpace();

        if (m_contentBrowser)
        {
            m_contentBrowser->Draw();
            // Draw all open texture editors, then remove any that were closed
            for (auto& textureEditor : m_textureEditors)
                textureEditor->Draw();
            m_textureEditors.erase(
                std::remove_if(m_textureEditors.begin(), m_textureEditors.end(),
                    [](const std::unique_ptr<TextureEditor>& textureEditor)
                    { return !textureEditor->IsOpen(); }),
                m_textureEditors.end());
            // Draw all open mesh viewers, then remove closed ones
            for (auto& meshViewer : m_meshViewers)
                meshViewer->Draw();
            m_meshViewers.erase(
                std::remove_if(m_meshViewers.begin(), m_meshViewers.end(),
                    [](const std::unique_ptr<MeshViewer>& meshViewer) 
                    { return !meshViewer->IsOpen(); }),
                m_meshViewers.end());
            // Draw all open sound players, then remove closed ones
            for (auto& soundPlayer : m_soundPlayers)
                soundPlayer->Draw();
            m_soundPlayers.erase(
                std::remove_if(m_soundPlayers.begin(), m_soundPlayers.end(),
                    [](const std::unique_ptr<SoundPlayer>& sp) 
                    { return !sp->IsOpen(); }),
                m_soundPlayers.end());
        }

        if (m_viewport)
            m_viewport->Draw();

        if (m_hierarchy)
            m_hierarchy->Draw();

        if (m_componentsViewer)
            m_componentsViewer->Draw();

        m_gui->EndFrame();

        m_window->SwapBuffers();
    }

    void Editor::Update()
    {
        m_window->UpdateDeltaTime();
        m_viewport->UpdateCamera(m_window.get());

        switch (m_editorState)
        {
        case Apex::Editor::EditorState::Edit:
            break;
        case Apex::Editor::EditorState::Play:
            m_app->Tick();
            break;
        case Apex::Editor::EditorState::Pause:
            break;
        case Apex::Editor::EditorState::Step:
            m_app->Tick();
            m_editorState = EditorState::Pause;
            break;
        default:
            break;
        }
    }

    void Editor::Shutdown()
    {
        m_textureEditors.clear();
        m_meshViewers.clear();
        m_soundPlayers.clear();
        m_contentBrowser.reset();
        m_hierarchy.reset();
        m_viewport.reset();
        EditorIcon::Shutdown();
        m_gui.reset();
        m_window.reset();
    }

    void Editor::SetEditorState(EditorState newState)
    {
        if (m_editorState == newState)
            return;

        
        if (m_editorState == EditorState::Edit && newState == EditorState::Play)
        {
            m_runtimeScene = m_editScene->Clone();

            m_app->SetScene(m_runtimeScene.get());

            m_viewport->ChangeScene(m_runtimeScene.get());
            m_hierarchy->SetScene(m_runtimeScene.get());

            m_runtimeScene->Start();
        }
        
        else if (newState == EditorState::Edit)
        {
            m_runtimeScene.reset();

            m_app->SetScene(m_editScene.get());

            m_viewport->ChangeScene(m_editScene.get());
            m_hierarchy->SetScene(m_editScene.get());
        }

        m_editorState = newState;
    }

    const char* Editor::TypeToString(AssetType type)
    {
        switch (type)
        {
        case AssetType::Mesh:    return "Mesh";
        case AssetType::Texture: return "Texture";
        case AssetType::Sound:   return "Sound";
        case AssetType::Level:   return "Level";
        case AssetType::Script:  return "Script";
        case AssetType::Folder:  return "Folder";
        default:                 return "Unknown";
        }
    }
    void Editor::OnExternalDrop(std::vector<std::filesystem::path> paths)
    {
        if (!m_contentBrowser) return;

        auto  mouse = m_window->GetMousePosition();
        float mouseX = static_cast<float>(mouse.m_x);
        float mouseY = static_cast<float>(mouse.m_y);

        fs::path dest = m_contentBrowser->GetExternalDropDest(mouseX, mouseY);
        if (!dest.empty())
        {
            m_contentBrowser->ImportFiles(std::move(paths), dest);
            return;
        }

        if (m_contentBrowser->ContainsGridPoint(mouseX, mouseY))
            m_contentBrowser->ImportFiles(std::move(paths));
    }

    void Editor::OpenTextureEditor(const fs::path& path)
    {
        // If this texture is already open, just focus that window
        for (auto& textureEditor : m_textureEditors)
        {
            if (textureEditor->GetPath() == path)
            {
                textureEditor->Focus();
                return;
            }
        }
        // Otherwise open a new editor for this texture
        auto textureEditor = std::make_unique<TextureEditor>(*m_gui, *m_window);
        textureEditor->Open(path, static_cast<int>(m_textureEditors.size()));
        m_textureEditors.push_back(std::move(textureEditor));
    }

    void Editor::OpenMeshViewer(const fs::path& path)
    {
        // If this mesh is already open, just focus that window
        for (auto& meshViewer : m_meshViewers)
        {
            if (meshViewer->GetPath() == path)
            {
                meshViewer->Focus();
                return;
            }
        }
        // Otherwise open a new viewer for this mesh
        auto meshViewer = std::make_unique<MeshViewer>(*m_gui, *m_window, *m_app->GetRhi(), *m_app->GetResourceManager());
        meshViewer->Open(path, static_cast<int>(m_meshViewers.size()));
        m_meshViewers.push_back(std::move(meshViewer));
    }

    void Editor::OpenSoundPlayer(const fs::path& path)
    {
        for (auto& soundPlayer : m_soundPlayers)
        {
            if (soundPlayer->GetPath() == path)
            {
                soundPlayer->Focus();
                return;
            }
        }
        auto soundPlayer = std::make_unique<SoundPlayer>(*m_gui, *m_window, *m_audio);
        soundPlayer->Open(path, static_cast<int>(m_soundPlayers.size()));
        m_soundPlayers.push_back(std::move(soundPlayer));
    }
}