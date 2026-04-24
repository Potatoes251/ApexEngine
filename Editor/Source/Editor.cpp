#include "Editor.h"

#include "EditorIcon.h"

#include "Shader.h"
#include "LogSystem.h"
#include "ResourceHandle.h"
#include "ResourceManager.h"

#include "Rigidbody.h"

#include "Audio.h"
#include "Scene.h"
#include "SceneLoader.h"
#include "CommandManager.h"
#include "MaterialEditor.h"

using namespace Apex::Data;
using namespace Apex::CtrlZ;
using namespace Apex::Audio;
using namespace Apex::Input;
using namespace Apex::Physic;
using namespace Apex::Rendering;
using namespace Apex::Windowing;
using namespace Apex::Serialization;
using namespace Apex::UserInterface;

namespace Apex::Editor 
{
    bool Editor::InitWindow()
    {
        m_window = std::unique_ptr<IWindow>(CreateWindow("Apex Editor", 1280, 720));
        m_gui = std::unique_ptr<IGUI>(CreateGUI(m_window->NativeHandle()));

        if (!m_window) return false;

        m_window->Maximize();

        m_window->SetCloseCallback([this]()
            {
                m_window->SetShouldClose(false);
                if (m_app->GetScene() && m_app->GetScene()->IsDirty())
                    m_confirmClosePopup = true;
                else
                    m_window->SetShouldClose(true);
            });

        return true;
    }

    bool Editor::InitPanels(Apex::Application& app)
    {
        m_app = &app;
        m_audio = app.GetAudio();

        if (!m_gui) return false;

        m_editScene = app.GetScene()->Clone();

        app.GetScene()->ClearDirty();

        app.GetScene()->SetOnMarkedDirtyCallback([this]()
            {
                m_window->SetWindowTitle("Apex Editor *");
            });

        m_console = std::make_unique<Console>(m_gui.get());
        LogSystem::AddSink(m_console.get());

        // All panels use m_editScene
        m_componentsViewer = std::make_unique<ComponentsViewer>(
            m_gui.get(),
            app.GetScene(),
            m_app->GetResourceManager(),
            app.GetScene()->GetPhysic(),
            m_app->GetLuaManager()
        );

        m_hierarchy = std::make_unique<Hierarchy>(
            *m_gui,
            app.GetScene(),
            m_componentsViewer.get(),
            *m_window
        );

        m_viewport = std::make_unique<Viewport>(
            app.GetResourceManager()->CreateAsync<Shader>("ApexAssets/Shaders/GizmoVert.glsl|ApexAssets/Shaders/GizmoFrag.glsl"),
            app.GetResourceManager()->CreateAsync<Shader>("ApexAssets/Shaders/GridVert.glsl|ApexAssets/Shaders/GridFrag.glsl"),
            app.GetRhi(),
            m_gui.get(),
            this
        );
        m_viewport->SetHierarchy(m_hierarchy.get());
        m_hierarchy->SetViewport(m_viewport.get());

        m_contentBrowser = std::make_unique<ContentBrowser>(*m_gui, *m_window, "Assets/" ,*app.GetRhi(), *app.GetResourceManager());

        if (m_componentsViewer && m_contentBrowser)
            m_componentsViewer->SetThumbnailRenderer(&m_contentBrowser->GetThumbnails());

        m_window->SetKeyCallback([&](Input::Key key, bool down)
            {
                if (key == Input::Key::P && down)
                {
                    m_app->GetScene()->Save("Assets/Scenes/scene.level");
                    m_window->SetWindowTitle("Apex Editor");
                }

                if (m_editorState == EditorState::Play && key == Input::Key::F && down)
                    m_viewport->ToggleFullScreen();

                if (key == Input::Key::Z && down && InputSystem::Get().IsKeyDown(Key::Ctrl))
                {
                    CommandManager::Get().Undo();
                }
                if (key == Input::Key::Y && down && InputSystem::Get().IsKeyDown(Key::Ctrl))
                {
                    CommandManager::Get().Redo();
                }

                if (key == Input::Key::S && down && InputSystem::Get().IsKeyDown(Key::Ctrl))
                    m_app->GetScene()->Save("Assets/Scenes/scene.level");
            });

        m_window->SetMouseMoveCallback([&](double mouseX, double mouseY)
            {
                if (m_editorState == EditorState::Play)
                {
                    Camera* mainCam = m_app->GetScene()->GetMainCamera();
                    if (mainCam)
                        mainCam->ProcessMouse(mouseX, mouseY);
                    else
                        m_viewport->GetCamera().ProcessMouse(mouseX, mouseY);
                }
                else if (InputSystem::Get().IsMouseButtonDown(Input::MouseButton::Right))
                {
                    if (m_viewport->IsFocus())
                        m_viewport->GetCamera().ProcessMouse(mouseX, mouseY);
                }
                else
                {
                    LibMath::Vector2 pos = InputSystem::Get().GetMousePos();
                    m_viewport->OnMouseMove(pos[0], pos[1]);
                }
            });

        m_window->SetMouseButtonCallback([&](Input::MouseButton button, bool press)
            {
                LibMath::Vector2 pos = InputSystem::Get().GetMousePos();
                if (button == Input::MouseButton::Left && press)
                {
                    m_viewport->OnMouseClick(pos[0], pos[1]);
                }
                else
                {
                    m_viewport->OnMouseRelease(pos[0], pos[1]);
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
                case AssetType::Hud:
                    OpenHudEditor(entry.m_path);
                    break;
                case AssetType::Material:
                    OpenMaterialEditor(entry.m_path);
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
            m_console->Draw();
            m_contentBrowser->Draw();
            // Draw all open texture editors, then remove any that were closed
            for (auto& textureEditor : m_textureEditors)
                textureEditor->Draw();
            m_textureEditors.erase(
                std::remove_if(m_textureEditors.begin(), m_textureEditors.end(),
                    [](const std::unique_ptr<TextureEditor>& textureEditor) { return !textureEditor->IsOpen(); }),
                m_textureEditors.end());
            // Draw all open mesh viewers, then remove closed ones
            for (auto& meshViewer : m_meshViewers)
                meshViewer->Draw();
            m_meshViewers.erase(
                std::remove_if(m_meshViewers.begin(), m_meshViewers.end(),
                    [](const std::unique_ptr<MeshViewer>& meshViewer) { return !meshViewer->IsOpen(); }),
                m_meshViewers.end());
            // Draw all open sound players, then remove closed ones
            for (auto& soundPlayer : m_soundPlayers)
                soundPlayer->Draw();
            m_soundPlayers.erase(
                std::remove_if(m_soundPlayers.begin(), m_soundPlayers.end(),
                    [](const std::unique_ptr<SoundPlayer>& sp) { return !sp->IsOpen(); }),
                m_soundPlayers.end());
            // Draw all open HUD editors, then remove closed ones
            for (auto& hudEditor : m_hudEditors)
                hudEditor->Draw();
            m_hudEditors.erase(
                std::remove_if(m_hudEditors.begin(), m_hudEditors.end(),
                    [](const std::unique_ptr<HudEditor>& he) { return !he->IsOpen(); }),
                m_hudEditors.end());

        }
        if (m_materialEditor)
            m_materialEditor->Draw();

        if (m_viewport)
            m_viewport->Draw();

        if (m_hierarchy)
            m_hierarchy->Draw();

        if (m_componentsViewer)
            m_componentsViewer->Draw();

        HandleEditorClosing();

        bool editing = m_gui->IsAnyItemActive();
        if (editing && !m_editingUI)
        {
            CommandManager::Get().StartBatch();
        }
        if (!editing && m_editingUI)
        {
            CommandManager::Get().EndBatch();
        }
        
        m_editingUI = editing;

        m_gui->EndFrame();

        m_window->SwapBuffers();
    }

    void Editor::Update()
    {
        m_window->UpdateDeltaTime();
        m_viewport->UpdateCamera(m_window.get());

        if (EditorState::Play != m_editorState)
            m_app->GetScene()->GetPhysic()->EditorSync();

        switch (m_editorState)
        {
        case EditorState::Edit:
            break;
        case EditorState::Play:
            m_app->Tick();
            if (InputSystem::Get().IsKeyDown(Key::Escape)) SetEditorState(EditorState::Pause);
            break;
        case EditorState::Pause:
            break;
        case EditorState::Step:
            m_app->Tick();
            m_editorState = EditorState::Pause;
            break;
        default:
            break;
        }
    }

    void Editor::HandleEditorClosing()
    {
        m_gui->PushFont(FontID::Default);

        if (m_confirmClosePopup)
        {
            m_gui->OpenPopup("ConfirmExit");
            m_confirmClosePopup = false;
        }

        if (m_gui->BeginPopupModal("ConfirmExit", { 400.f, 0.f }))
        {
            m_gui->Text("\nUnsaved changes.\nExit anyway?\n\n");

            if (m_gui->Button("Save & Exit"))
            {
                m_app->GetScene()->Save("Assets/Scenes/scene.level");
                m_window->SetShouldClose(true);
                m_gui->CloseCurrentPopup();
            }

            m_gui->SameLine();

            if (m_gui->Button("Exit"))
            {
                m_window->SetShouldClose(true);
                m_gui->CloseCurrentPopup();
            }

            m_gui->SameLine();

            if (m_gui->Button("Cancel"))
                m_gui->CloseCurrentPopup();

            m_gui->EndPopup();
        }

        m_gui->PopFont();
    }

    void Editor::Shutdown()
    {
        m_textureEditors.clear();
        m_meshViewers.clear();
        m_soundPlayers.clear();
        m_hudEditors.clear();
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

        // Always clear selection before destroying old scene objects
        if (m_componentsViewer) m_componentsViewer->ClearSelection();
        if (m_hierarchy)        m_hierarchy->ClearSelection();
        
        // start
        if (m_editorState == EditorState::Edit && newState == EditorState::Play)
        {
            m_editScene = m_app->GetScene()->Clone();
            m_app->SetScene(m_editScene->Clone());

            auto* ui = m_app->GetUI();
            if (ui)
            {
                ui->ClearAll();
                ui->ScanHUDs("Assets/");
            }

            m_app->GetScene()->Start();

            if (m_hierarchy) m_hierarchy->SetScene(m_app->GetScene());
            if (m_componentsViewer) m_componentsViewer->SetScene(m_app->GetScene());
        }
        // stop
        else if (newState == EditorState::Edit)
        {
            m_app->SetScene(m_editScene->Clone());
            if (m_hierarchy) m_hierarchy->SetScene(m_app->GetScene());
            if (m_componentsViewer) m_componentsViewer->SetScene(m_app->GetScene());
            auto* ui = m_app->GetUI();
            if (ui)
                ui->ClearAll();
        }

        // capture cursor when in play uncapture it when not in play
        m_window->SetCursorCaptured(newState == EditorState::Play);

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
        case AssetType::Hud:     return "HUD";
        case AssetType::Material:     return "Material";
        case AssetType::Folder:  return "Folder";
        default:                 return "Unknown";
        }
    }

    void Editor::OpenTextureEditor(const Fs::path& path)
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

    void Editor::OpenMeshViewer(const Fs::path& path)
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
        auto meshViewer = std::make_unique<MeshViewer>(m_gui.get(), m_app->GetRhi(), m_app->GetResourceManager());
        meshViewer->Open(path, static_cast<int>(m_meshViewers.size()));
        m_meshViewers.push_back(std::move(meshViewer));
    }

    void Editor::OpenSoundPlayer(const Fs::path& path)
    {
        for (auto& soundPlayer : m_soundPlayers)
        {
            if (soundPlayer->GetPath() == path)
            {
                soundPlayer->Focus();
                return;
            }
        }
        auto soundPlayer = std::make_unique<SoundPlayer>(m_gui.get(), m_audio);
        soundPlayer->Open(path, static_cast<int>(m_soundPlayers.size()));
        m_soundPlayers.push_back(std::move(soundPlayer));
    }

    void Editor::OpenHudEditor(const Fs::path& path)
    {
        for (auto& hudEditor : m_hudEditors)
        {
            if (hudEditor->GetPath() == path)
            {
                hudEditor->Focus();
                return;
            }
        }
        auto hudEditor = std::make_unique<HudEditor>(m_gui.get(), m_app->GetResourceManager(), m_app->GetRhi());
        hudEditor->Open(path, static_cast<int>(m_hudEditors.size()));
        m_hudEditors.push_back(std::move(hudEditor));
    }

    void Editor::OpenMaterialEditor(const std::filesystem::path& path)
    {
        m_materialEditor = std::make_unique<MaterialEditor>(m_gui.get(), m_app->GetResourceManager());
        m_materialEditor->Open(path.string());
    }
}