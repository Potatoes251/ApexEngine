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
#include "Log.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>
#include <shellapi.h> // only needed for opening Visual Studio when opening a script from the editor
#undef CreateWindow
#endif

using namespace Apex::Windowing;
using namespace Apex::UserInterface;
using namespace Apex::Data;
using namespace Apex::CtrlZ;
using namespace Apex::Audio;
using namespace Apex::Input;
using namespace Apex::Physic;
using namespace Apex::Rendering;
using namespace Apex::Windowing;
using namespace Apex::Serialization;
using namespace Apex::Physic;
using namespace Apex::Input;
using namespace Apex::Scripting;
using namespace Apex::UserInterface;

namespace Apex::Editor
{
    bool Editor::InitWindow()
    {
        m_window = std::unique_ptr<IWindow>(CreateWindow("Apex Editor", 1280, 720));
        m_gui = std::unique_ptr<IGUI>(CreateGUI(m_window->NativeHandle()));

        if (!m_window) return false;

        m_window->SetWindowIcon("ApexAssets/Textures/EditorIcon.png");

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

        m_componentsViewer = std::make_unique<ComponentsViewer>(
            m_gui.get(),
            app.GetScene(),
            m_app->GetResourceManager(),
            m_app->GetLuaManager()
        );

        m_hierarchy = std::make_unique<Hierarchy>(
            *m_gui,
            app.GetScene(),
            m_componentsViewer.get(),
            *m_window,
            app.GetResourceManager()
        );

        m_viewport = std::make_unique<Viewport>(
            app.GetResourceManager()->CreateAsync<Shader>("ApexAssets/Shaders/GizmoVert.glsl|ApexAssets/Shaders/GizmoFrag.glsl"),
            app.GetResourceManager()->CreateAsync<Shader>("ApexAssets/Shaders/GridVert.glsl|ApexAssets/Shaders/GridFrag.glsl"),
            app.GetRhi(),
            m_gui.get(),
            this
        );

        LoadCameraSettings();

        m_viewport->SetHierarchy(m_hierarchy.get());
        m_hierarchy->SetViewport(m_viewport.get());

        m_contentBrowser = std::make_unique<ContentBrowser>(*m_gui, *m_window, "Assets/", *app.GetRhi(), *app.GetResourceManager());

        if (m_componentsViewer && m_contentBrowser)
            m_componentsViewer->SetThumbnailRenderer(&m_contentBrowser->GetThumbnails());

        m_window->SetKeyCallback([&](Input::Key key, bool down)
            {
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
                if (key == Input::Key::C && down && InputSystem::Get().IsKeyDown(Key::Ctrl))
                {
                    CopySelected();
                }
                if (key == Input::Key::V && down && InputSystem::Get().IsKeyDown(Key::Ctrl))
                {
                    PasteClipboard();
                }
                if (key == Input::Key::D && down && InputSystem::Get().IsKeyDown(Key::Ctrl))
                {
                    DuplicateSelected();
                }

                if (key == Input::Key::S && down && InputSystem::Get().IsKeyDown(Key::Ctrl))
                {
                    m_app->GetScene()->Save();
                    m_window->SetWindowTitle("Apex Editor");
                }
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

        m_window->SetScrollCallback([&](double /*xoffset*/, double yoffset)
            {
                if (m_editorState != EditorState::Play
                    && m_viewport->IsFocus()
                    && InputSystem::Get().IsMouseButtonDown(Input::MouseButton::Right))
                {
                    m_viewport->GetCamera().ProcessScroll(yoffset);
                    return;
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
                case AssetType::Level:
                {
                    if (m_editorState == EditorState::Play)
                        SetEditorState(EditorState::Edit);

                    OnSceneChanged();
                    m_app->LoadScene(entry.m_path.string(), false);

                    CleanClipboard();

                    // Sync panels
                    if (m_hierarchy)        m_hierarchy->SetScene(m_app->GetScene());
                    if (m_componentsViewer) m_componentsViewer->SetScene(m_app->GetScene());

                    m_editScene = m_app->GetScene()->Clone();
                    m_app->GetScene()->ClearDirty();

                    break;
                }
                case AssetType::Script:
                {
                    std::filesystem::path absolutePath = std::filesystem::absolute(entry.m_path);

					// Open Visual Studio with the script file
                    ShellExecuteW(NULL, L"open", L"devenv.exe",absolutePath.c_str(), NULL, SW_SHOWNORMAL);
                    break;
                }
                default:
                    break;
                }
            });
        
        return true;
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
                m_app->GetScene()->Save();
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
        SaveUserSettings();

        m_textureEditors.clear();
        m_meshViewers.clear();
        m_soundPlayers.clear();
        m_hudEditors.clear();
        m_materialEditor.reset();
        m_contentBrowser.reset();
        m_hierarchy.reset();
        m_viewport.reset();
        EditorIcon::Shutdown();
        m_gui.reset();
        m_window.reset();
        m_editScene.reset();
    }

    void Editor::SetEditorState(EditorState newState)
    {
        if (m_editorState == newState)
            return;

        // Always clear selection before destroying old scene objects
        if (m_componentsViewer) m_componentsViewer->ClearSelection();
        if (m_hierarchy)        m_hierarchy->ClearSelection();

        if (newState == EditorState::Play || newState == EditorState::Edit)
            OnSceneChanged();       // clear stale pointers before scene swap

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
            m_app->GetAudio()->StopAll();
            m_app->Unpause();
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

    void Editor::CollectAssetPaths(const std::string& scenePath,
        std::unordered_set<std::string>& outAssets,
        std::unordered_set<std::string>& outMats)
    {
        std::ifstream file(scenePath);
        if (!file.is_open())
        {
            LOG_WARNING_CAT("Build", "Could not open scene {}", scenePath.c_str());
            return;
        }

        std::string text((std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>());

        Serialization::SerialParser parser(text);

        // Walk every key-value pair in the JSON.
        // When a string value starts with "Assets/", collect it.
        // Shader paths are pipe-separated ("a.vert|b.frag") so split those too.
        // We don't need to understand the schema — just find all strings.
        std::function<void()> collectValues;
        collectValues = [&]()
            {
                char c = parser.PeekChar();

                if (c == '"')
                {
                    std::string val = parser.ParseString();

                    // Split on '|' for shader paths
                    std::stringstream ss(val);
                    std::string part;
                    while (std::getline(ss, part, '|'))
                    {
                        if (part.starts_with("Assets") || part.starts_with("ApexAssets/"))
                        {
                            outAssets.insert(part);

                            if (part.ends_with(".mat"))
                                outMats.insert(part);
                        }
                    }
                }
                else if (c == '{')
                {
                    parser.Expect('{');
                    if (parser.Peek('}')) { parser.Expect('}'); return; }

                    while (true)
                    {
                        parser.ParseString(); // key — discard
                        parser.Expect(':');
                        collectValues();      // value — recurse

                        if (parser.Peek(',')) { parser.Expect(','); continue; }
                        break;
                    }
                    parser.Expect('}');
                }
                else if (c == '[')
                {
                    parser.Expect('[');
                    if (parser.Peek(']')) { parser.Expect(']'); return; }

                    while (true)
                    {
                        collectValues();

                        if (parser.Peek(',')) { parser.Expect(','); continue; }
                        break;
                    }
                    parser.Expect(']');
                }
                else
                {
                    // number, bool, or null — consume until delimiter
                    // ParseFloat handles numbers; Match handles true/false/null
                    if (c == 't' || c == 'f') parser.ParseBool();
                    else if (c == 'n')        parser.Match("null");
                    else                      parser.ParseFloat();
                }
            };

        try { collectValues(); }
        catch (const std::exception& e)
        {
            LOG_WARNING_CAT("Build", "Failed to parse {}: {}", scenePath.c_str(), e.what());
        }
    }

    void Editor::CollectLuaDependencies(const std::string& scriptPath,
        std::unordered_set<std::string>& out,
        std::unordered_set<std::string>& visited)
    {
        if (!visited.insert(scriptPath).second) // already visited
            return;

        std::ifstream file(Fs::current_path() / scriptPath);
        if (!file.is_open())
        {
            LOG_WARNING_CAT("Build", "Could not open script ", scriptPath.c_str());
            return;
        }

        std::string line;
        while (std::getline(file, line))
        {
            auto commentPos = line.find("--");
            if (commentPos != std::string::npos)
                line = line.substr(0, commentPos);

            size_t pos = 0;
            while ((pos = line.find("require", pos)) != std::string::npos)
            {
                pos += 7;
                while (pos < line.size() && line[pos] == ' ') ++pos;
                if (pos >= line.size() || line[pos] != '(') continue;
                ++pos;
                while (pos < line.size() && line[pos] == ' ') ++pos;
                if (pos >= line.size() || (line[pos] != '"' && line[pos] != '\'')) continue;
                char quote = line[pos++];
                size_t nameStart = pos;
                while (pos < line.size() && line[pos] != quote) ++pos;
                if (pos >= line.size()) continue;

                std::string moduleName = line.substr(nameStart, pos - nameStart);
                if (moduleName.empty()) continue;

                std::string modulePath = "Assets/Scripts/" + moduleName + ".lua";
                out.insert(modulePath);
                CollectLuaDependencies(modulePath, out, visited);
            }
        }
    }

    void Editor::CollectHudDependencies(
        const std::string& scriptPath,
        std::unordered_set<std::string>& outHuds)
    {
        std::ifstream file(Fs::current_path() / scriptPath);

        if (!file.is_open())
            return;

        std::string line;

        while (std::getline(file, line))
        {
            auto commentPos = line.find("--");
            if (commentPos != std::string::npos)
                line = line.substr(0, commentPos);

            size_t pos = 0;

            while ((pos = line.find("CreateHUD", pos)) != std::string::npos)
            {
                pos += strlen("CreateHUD");

                while (pos < line.size() && std::isspace(line[pos]))
                    ++pos;

                if (pos >= line.size() || line[pos] != '(')
                    continue;

                ++pos;

                while (pos < line.size() && std::isspace(line[pos]))
                    ++pos;

                if (pos >= line.size())
                    continue;

                if (line[pos] != '"' && line[pos] != '\'')
                    continue;

                char quote = line[pos++];

                size_t start = pos;

                while (pos < line.size() && line[pos] != quote)
                    ++pos;

                if (pos >= line.size())
                    continue;

                std::string hudName = line.substr(start, pos - start);

                outHuds.insert("Assets/HUD/" + hudName + ".hud");
            }
        }
    }

    void Editor::CollectAssetsFromMats(const std::string& matPath,
        std::unordered_set<std::string>& outAssets)
    {
        std::ifstream file(matPath);
        if (!file.is_open())
        {
            LOG_WARNING_CAT("Build", "Could not open material ", matPath.c_str());
            return;
        }

        std::string text((std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>());

        Serialization::SerialParser parser(text);

        std::function<void()> collectValues;
        collectValues = [&]()
            {
                char c = parser.PeekChar();

                if (c == '"')
                {
                    std::string val = parser.ParseString();

                    // Split shaders "a.vert|b.frag"
                    std::stringstream ss(val);
                    std::string part;
                    while (std::getline(ss, part, '|'))
                    {
                        if (part.starts_with("Assets/") || part.starts_with("ApexAssets/"))
                            outAssets.insert(part);
                    }
                }
                else if (c == '{')
                {
                    parser.Expect('{');
                    if (parser.Peek('}'))
                    {
                        parser.Expect('}');
                        return;
                    }

                    while (true)
                    {
                        if (parser.PeekChar() == '}') break; // handles trailing content before closing brace

                        if (parser.PeekChar() == '"')
                        {
                            std::string possible = parser.ParseString();

                            if (parser.Peek(':'))
                            {
                                parser.Expect(':');
                                collectValues();
                            }
                            else
                            {
                                // No colon, treat as a bare value (malformed object used as array)
                                if (possible.starts_with("Assets/") || possible.starts_with("ApexAssets/"))
                                    outAssets.insert(possible);
                            }
                        }
                        else
                        {
                            collectValues();
                        }

                        if (parser.Peek(','))
                        {
                            parser.Expect(',');
                            continue;
                        }
                        break;
                    }

                    parser.Expect('}');
                }
                else if (c == '[')
                {
                    parser.Expect('[');
                    if (parser.Peek(']'))
                    {
                        parser.Expect(']');
                        return;
                    }

                    while (true)
                    {
                        collectValues();

                        if (parser.Peek(','))
                        {
                            parser.Expect(',');
                            continue;
                        }
                        break;
                    }

                    parser.Expect(']');
                }
                else
                {
                    if (c == 't' || c == 'f') parser.ParseBool();
                    else if (c == 'n')        parser.Match("null");
                    else                      parser.ParseFloat();
                }
            };

        try { collectValues(); }
        catch (const std::exception& e)
        {
            LOG_WARNING_CAT("Build", "Failed to parse {}: {}", matPath.c_str(), e.what());
        }
    }

    void Editor::Build(const std::vector<std::string>& scenePaths)
    {
        namespace Fs = std::filesystem;

        Fs::path gameExeDir = Fs::path(GAME_EXE_DIR);
        Fs::path outputDir = Fs::current_path() / "ApexBuild";

        BuildSetupOutputDir(outputDir);
        BuildCopyExecutables(gameExeDir, outputDir);
        BuildCopyFonts(outputDir);
        BuildCopyEngineAssets(outputDir);
        BuildCopySceneFiles(scenePaths, outputDir);

        std::unordered_set<std::string> assetsToCopy;
        BuildCollectDependencies(scenePaths, assetsToCopy);
        BuildCopyAssets(assetsToCopy, outputDir);

        BuildWriteScenesJson(scenePaths, outputDir);
    }

    // ---------------------------------------------------------------------------

    void Editor::BuildSetupOutputDir(const Fs::path& outputDir)
    {
        std::error_code ec;

        if (Fs::exists(outputDir))
            Fs::remove_all(outputDir, ec);

        if (ec)
        {
            LOG_ERROR_CAT("Build", "Could not clear previous build: {}", ec.message().c_str());
            return;
        }

        Fs::create_directories(outputDir, ec);
    }

    void Editor::BuildCopyExecutables(const Fs::path& gameExeDir, const Fs::path& outputDir)
    {
        std::error_code ec;

        Fs::copy(gameExeDir / "GameLauncher.exe", outputDir / "Game.exe",
            Fs::copy_options::overwrite_existing, ec);

        for (auto& entry : Fs::directory_iterator(gameExeDir, ec))
        {
            if (entry.path().extension() == ".dll")
                Fs::copy(entry.path(), outputDir / entry.path().filename(),
                    Fs::copy_options::overwrite_existing, ec);
        }
    }

    void Editor::BuildCopyFonts(const Fs::path& outputDir)
    {
        std::error_code ec;

        Fs::path src = Fs::current_path() / "ApexAssets/Fonts";
        Fs::path dst = outputDir / "ApexAssets/Fonts";

        Fs::create_directories(dst, ec);

        for (auto& entry : Fs::recursive_directory_iterator(src))
        {
            const auto& path = entry.path();
            auto target = dst / Fs::relative(path, src);

            if (entry.is_directory())
            {
                Fs::create_directories(target, ec);
            }
            else
            {
                Fs::create_directories(target.parent_path(), ec);
                Fs::copy_file(path, target, Fs::copy_options::overwrite_existing, ec);
            }

            if (ec)
            {
                LOG_ERROR_CAT("Build", "Copy error: {}", ec.message().c_str());
                ec.clear();
            }
        }
    }

    void Editor::BuildCopyEngineAssets(const Fs::path& outputDir)
    {
        static const char* s_engineAssets[] =
        {
            "ApexAssets/Shaders/DirShadowVert.glsl",
            "ApexAssets/Shaders/DirShadowFrag.glsl",
            "ApexAssets/Shaders/OmniShadowVert.glsl",
            "ApexAssets/Shaders/OmniShadowFrag.glsl",
            "ApexAssets/Shaders/OmniShadowGeo.glsl",
            "ApexAssets/Shaders/SkyboxVert.glsl",
            "ApexAssets/Shaders/SkyboxFrag.glsl",
            "ApexAssets/Textures/Skybox/px.png",
            "ApexAssets/Textures/Skybox/nx.png",
            "ApexAssets/Textures/Skybox/ny.png",
            "ApexAssets/Textures/Skybox/py.png",
            "ApexAssets/Textures/Skybox/pz.png",
            "ApexAssets/Textures/Skybox/nz.png",
            "ApexAssets/Meshes/cube/cube.mesh",
        };

        std::error_code ec;

        for (const char* assetPath : s_engineAssets)
        {
            Fs::path src = Fs::current_path() / assetPath;
            Fs::path dst = outputDir / assetPath;
            Fs::create_directories(dst.parent_path(), ec);
            Fs::copy(src, dst, Fs::copy_options::overwrite_existing, ec);
        }
    }

    void Editor::BuildCopySceneFiles(const std::vector<std::string>& scenePaths, const Fs::path& outputDir)
    {
        std::error_code ec;

        for (const auto& scenePath : scenePaths)
        {
            Fs::path src = Fs::current_path() / scenePath;
            Fs::path dst = outputDir / scenePath;
            Fs::create_directories(dst.parent_path(), ec);
            Fs::copy(src, dst, Fs::copy_options::overwrite_existing, ec);

            // .nav sidecar
            src = src.replace_extension(".nav");
            dst = dst.replace_extension(".nav");
            Fs::copy(src, dst, Fs::copy_options::overwrite_existing, ec);
        }
    }

    void Editor::BuildCollectDependencies(const std::vector<std::string>& scenePaths,
        std::unordered_set<std::string>& outAssets)
    {
        std::unordered_set<std::string> matsToParse;

        // 1. Scene JSON → asset paths + material paths
        for (const auto& scenePath : scenePaths)
            CollectAssetPaths(scenePath, outAssets, matsToParse);

        // 2. Lua scripts → HUD dependencies
        std::unordered_set<std::string> hudDeps;
        for (const auto& assetPath : outAssets)
            if (assetPath.ends_with(".lua"))
                CollectHudDependencies(assetPath, hudDeps);
        outAssets.merge(hudDeps);

        // 3. HUD files → texture dependencies
        Fs::path hudDir = Fs::current_path() / "Assets/HUD";
        if (Fs::exists(hudDir))
        {
            for (const auto& entry : Fs::recursive_directory_iterator(hudDir))
            {
                if (!entry.is_regular_file() || entry.path().extension() != ".hud")
                    continue;

                std::ifstream file(entry.path());
                std::string line;
                while (std::getline(file, line))
                {
                    size_t texPos = line.find("\"texture\"");
                    if (texPos == std::string::npos) continue;

                    size_t q1 = line.find('"', texPos + 9);
                    if (q1 == std::string::npos) continue;

                    size_t q2 = line.find('"', q1 + 1);
                    if (q2 == std::string::npos) continue;

                    outAssets.insert(line.substr(q1 + 1, q2 - q1 - 1));
                }
            }
        }

        // 4. Material files themselves
        for (const auto& matPath : matsToParse)
            outAssets.insert(matPath);

        // 5. Lua scripts → transitive require() dependencies
        std::unordered_set<std::string> luaDeps;
        std::unordered_set<std::string> visited;
        for (const auto& assetPath : outAssets)
            if (assetPath.ends_with(".lua"))
                CollectLuaDependencies(assetPath, luaDeps, visited);
        outAssets.merge(luaDeps);

        // 6. Material files → referenced textures/shaders
        std::unordered_set<std::string> matDeps;
        for (const auto& assetPath : outAssets)
            if (assetPath.ends_with(".mat"))
                CollectAssetsFromMats(assetPath, matDeps);
        outAssets.merge(matDeps);
    }

    void Editor::BuildCopyAssets(const std::unordered_set<std::string>& assets,
        const Fs::path& outputDir)
    {
        constexpr auto copyOpts = Fs::copy_options::overwrite_existing
            | Fs::copy_options::recursive;
        std::error_code ec;

        for (const auto& assetPath : assets)
        {
            Fs::path src = Fs::current_path() / assetPath;
            Fs::path dst = outputDir / assetPath;
            Fs::create_directories(dst.parent_path(), ec);
            Fs::copy(src, dst, Fs::copy_options::overwrite_existing, ec);

            // Meshes may have a sibling Animation/ folder
            if (src.extension() == ".mesh")
            {
                Fs::path animSrc = src.parent_path() / "Animation";
                if (Fs::exists(animSrc) && Fs::is_directory(animSrc))
                {
                    Fs::path animDst = dst.parent_path() / "Animation";
                    Fs::copy(animSrc, animDst, copyOpts, ec);
                    if (ec)
                        LOG_ERROR_CAT("Build", "Could not copy Animation folder for {}: {}",
                            assetPath.c_str(), ec.message().c_str());
                }
            }
        }
    }

    void Editor::BuildWriteScenesJson(const std::vector<std::string>& scenePaths,
        const Fs::path& outputDir)
    {
        std::error_code ec;

        Fs::path jsonDst = outputDir / "Assets/build_scenes.json";
        Fs::create_directories(jsonDst.parent_path(), ec);

        std::ofstream json(jsonDst);
        if (!json.is_open())
        {
            LOG_ERROR_CAT("Build", "Failed to write build_scenes.json");
            return;
        }

        json << "{\n    \"scenes\": [\n";
        for (size_t i = 0; i < scenePaths.size(); ++i)
        {
            std::string path = scenePaths[i];
            std::replace(path.begin(), path.end(), '\\', '/');
            json << "        \"" << path << "\"";
            if (i + 1 < scenePaths.size()) json << ",";
            json << "\n";
        }
        json << "    ]\n}\n";

        LOG_INFO_CAT("Build", "build_scenes.json written ({} scene(s))", scenePaths.size());
    }

    void Editor::OpenMaterialEditor(const std::filesystem::path& path)
    {
        m_materialEditor = std::make_unique<MaterialEditor>(m_gui.get(), m_app->GetResourceManager());
        m_materialEditor->SetThumbnailRenderer(&m_contentBrowser->GetThumbnails());
        m_materialEditor->Open(path.string());
    }

    // ---------------------------------------------------------------------------
    // Clipboard shortcuts — Ctrl+C / Ctrl+V / Ctrl+D
    //
    // Priority order for focus: Viewport > Hierarchy > ContentBrowser.
    // ContentBrowser copy/paste operates on files; Hierarchy/Viewport operate on
    // scene objects.  Only one "clipboard slot" is live at a time: copying an
    // object clears the file clipboard and vice-versa.
    // ---------------------------------------------------------------------------

    void Editor::CopySelected()
    {
        if (m_hierarchy && m_hierarchy->IsFocused())
        {
            Data::Object* obj = m_hierarchy->GetSelected();
            if (obj)
            {
                m_clipboardObjectId = obj->GetId();
                m_hasClipboardObject = true;
            }
        }
        else if (m_viewport && m_viewport->IsFocus())
        {
            // Viewport stores its own selected pointer — grab it via hierarchy
            Data::Object* obj = m_hierarchy ? m_hierarchy->GetSelected() : nullptr;
            if (obj)
            {
                m_clipboardObjectId = obj->GetId();
                m_hasClipboardObject = true;
            }
        }
    }

    void Editor::PasteClipboard()
    {
        // --- object paste ---
        if (m_hasClipboardObject)
        {
            // Validate the id still exists in the CURRENT scene before touching it
            Data::Object* src = m_app->GetScene()->GetObjectWithId(m_clipboardObjectId);
            if (!src)
            {
                CleanClipboard();   // stale — silently discard
                return;
            }
            if (m_hierarchy && m_hierarchy->IsFocused())
                m_hierarchy->PasteObject(src);
        }
        // --- file paste ---
        else if (!m_clipboardFile.empty())
        {
            if (m_contentBrowser)
                m_contentBrowser->PasteFile(m_clipboardFile);
        }
    }

    void Editor::DuplicateSelected()
    {
        // Viewport focus — duplicate the currently selected object
        if ((m_viewport && m_viewport->IsFocus()) ||
            (m_hierarchy && m_hierarchy->IsFocused()))
        {
            if (m_hierarchy)
                m_hierarchy->DuplicateSelected();
            return;
        }

        // ContentBrowser focused — duplicate selected file in-place
        if (m_contentBrowser && m_contentBrowser->IsFocused())
        {
            m_contentBrowser->DuplicateSelected();
        }
    }
    void Editor::CleanClipboard()
    {
        m_hasClipboardObject = false;
        m_clipboardObjectId = 0;
        m_clipboardFile.clear();
    }
    
    void Editor::OnSceneChanged()
    {
        // Clear all raw-pointer state that references the old scene's objects
        CleanClipboard();
        if (m_hierarchy)
            m_hierarchy->ClearSelection();
        if (m_componentsViewer)
            m_componentsViewer->SetSelectedObject(nullptr);
        if (m_viewport)
            m_viewport->SelectObject(nullptr, false);
    }

    void Editor::LoadCameraSettings()
    {
        std::ifstream file(m_app->GetPrefsPath());

        if (!file.is_open())
            return;

        std::string line;

        while (std::getline(file, line))
        {
            if (line.starts_with("CameraPosition="))
            {
                size_t start = line.find('{');
                size_t end = line.find('}');

                if (start != std::string::npos && end != std::string::npos)
                {
                    std::string values = line.substr(start + 1, end - start - 1);

                    float x, y, z;

                    if (sscanf_s(values.c_str(), "%ff, %ff, %ff", &x, &y, &z) == 3)
                        m_viewport->GetCamera().SetPosition({ x, y, z });
                }
            }

            else if (line.starts_with("CameraSpeed="))
            {
                size_t equal = line.find('=');
                size_t semicolon = line.find(';');

                if (equal != std::string::npos)
                {
                    std::string value = line.substr(equal + 1, semicolon - equal - 1);

                    float speed;

                    if (sscanf_s(value.c_str(), "%ff", &speed) == 1)
                        m_viewport->GetCamera().SetSpeed(speed);
                }
            }

            else if (line.starts_with("CameraYaw="))
            {
                size_t equal = line.find('=');
                size_t semicolon = line.find(';');
                if (equal != std::string::npos)
                {
                    std::string value = line.substr(equal + 1, semicolon - equal - 1);
                    float yaw;
                    if (sscanf_s(value.c_str(), "%ff", &yaw) == 1)
                        m_viewport->GetCamera().SetOrientation(
                            m_viewport->GetCamera().GetPitch(), LibMath::Degree(yaw));
                }
            }
            else if (line.starts_with("CameraPitch="))
            {
                size_t equal = line.find('=');
                size_t semicolon = line.find(';');
                if (equal != std::string::npos)
                {
                    std::string value = line.substr(equal + 1, semicolon - equal - 1);
                    float pitch;
                    if (sscanf_s(value.c_str(), "%ff", &pitch) == 1)
                        m_viewport->GetCamera().SetOrientation(
                            LibMath::Degree(pitch), m_viewport->GetCamera().GetYaw());
                }
            }
        }
    }

    void Editor::SaveUserSettings()
    {
        std::filesystem::create_directories("Editor");

        // Read existing file
        std::vector<std::string> lines;
        {
            std::ifstream inputFile(m_app->GetPrefsPath());
            std::string line;
            while (std::getline(inputFile, line))
                lines.push_back(line);
        }

        // Helper: update a line in-place, or append it if missing
        auto upsert = [&](std::string_view key, std::string value, bool prepend = false)
            {
                for (auto& l : lines)
                {
                    if (l.starts_with(key))
                    {
                        l = std::string(key) + value;
                        return;
                    }
                }
                // Not found — insert
                if (prepend)
                    lines.insert(lines.begin(), std::string(key) + value);
                else
                    lines.push_back(std::string(key) + value);
            };

        const Camera& cam = m_viewport->GetCamera();
        LibMath::Vector3 pos = cam.GetPosition();

        upsert(m_app->GetLastSceneKey(), m_app->GetScene()->GetName(), true);
        upsert("CameraPosition=", "{"
            + std::to_string(pos[0]) + "f, "
            + std::to_string(pos[1]) + "f, "
            + std::to_string(pos[2]) + "f};");
        upsert("CameraSpeed=", std::to_string(cam.GetSpeed()) + "f;");
        upsert("CameraYaw=", std::to_string(cam.GetYaw().raw()) + "f;");
        upsert("CameraPitch=", std::to_string(cam.GetPitch().raw()) + "f;");

        std::ofstream outputFile(m_app->GetPrefsPath(), std::ios::trunc);
        for (const std::string& l : lines)
            outputFile << l << '\n';
    }
}