#include "Editor.h"

#include "Shader.h"
#include "ResourceHandle.h"

using namespace Apex::Windowing;
using namespace Apex::UserInterface;
using namespace Apex::Audio;

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
		m_audio = std::unique_ptr<IAudioEngine>(CreateAudioEngine());
        m_gui = std::unique_ptr<IGUI>(CreateGUI(m_window->NativeHandle()));

        if (!m_gui)
            return false;

        m_contentBrowser = std::make_unique<ContentBrowser>(*m_gui, *m_window, "Assets/");

        m_contentBrowser->SetOnAssetLoad([&](const AssetEntry& entry)
            {
                if (m_audio && entry.m_type == AssetType::Sound)
                    m_audio->Play2D(entry.m_path.filename().string());
            });

        m_viewport = std::make_unique<Viewport>(
            app.GetResourceManager()->CreateAsync<Shader>("Assets/Shaders/GizmoVert.glsl|Assets/Shaders/GizmoFrag.glsl"), 
            app.GetScene(), app.GetRhi(), m_gui.get());

        m_window->SetKeyCallback([&](Key key, KeyAction action)
            {
                if (key == Key::Escape && action == KeyAction::Press)
                    m_window->Close();

                if (key == Key::P && action == KeyAction::Press)
                    m_app->GetScene()->Save("Assets/Scenes/scene.level", *m_app->GetPhysicsSystem());
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
                default:
                    break;
                }
            });
        m_hierarchy = std::make_unique<Hierarchy>(*m_gui, *m_app->GetScene());

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

        m_gui->BeginFrame();

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
        }

        if (m_viewport)
            m_viewport->Draw();

        if (m_hierarchy)
            m_hierarchy->Draw();

        m_gui->EndFrame();

        m_window->SwapBuffers();
    }

    void Editor::Update()
    {
        m_window->UpdateDeltaTime();
        m_viewport->GetCamera().ProcessKeyboard(
            m_window->IsKeyPressed(Key::W),
            m_window->IsKeyPressed(Key::S),
            m_window->IsKeyPressed(Key::A),
            m_window->IsKeyPressed(Key::D),
            m_window->IsKeyPressed(Key::E),
            m_window->IsKeyPressed(Key::Q),
            m_window->GetDeltaTime()
        );
    }

    void Editor::Shutdown()
    {
        m_textureEditors.clear();
        m_contentBrowser.reset();
        m_viewport.reset();
        m_gui.reset();
        m_window.reset();
    }

    const char* Editor::TypeToString(AssetType type)
    {
        switch (type)
        {
        case AssetType::Mesh:    return "Mesh";
        case AssetType::Texture: return "Texture";
        case AssetType::Sound:   return "Sound";
        case AssetType::Level:   return "Level";
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
        auto textureEditor = std::make_unique<TextureEditor>(*m_gui);
        textureEditor->Open(path, static_cast<int>(m_textureEditors.size()));
        m_textureEditors.push_back(std::move(textureEditor));
    }
}