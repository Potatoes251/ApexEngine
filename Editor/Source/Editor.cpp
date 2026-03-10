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

        m_contentBrowser = std::make_unique<ContentBrowser>(*m_gui, "Assets/");

        m_contentBrowser->SetOnAssetLoad([&](const AssetEntry& entry)
            {
                if (m_audio && entry.m_type == AssetType::Sound)
                    m_audio->Play2D(entry.m_path.filename().string());
            });

        m_viewport = std::make_unique<Viewport>(
            app.GetResourceManager()->CreateAsync<Shader>("Assets/Shaders/GizmoVert.glsl|Assets/Shaders/GizmoFrag.glsl"), 
            app.GetScene(), app.GetRhi(), m_gui.get());

        m_window->SetMouseMoveCallback([&](double dx, double dy)
            {
                if (m_window->IsMousButtonPressed(Windowing::MouseButton::Right))
                    m_viewport->GetCamera().ProcessMouse(dx, dy);
            });

        m_window->MakeContextCurrent();

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

        if (m_gui && m_contentBrowser)
        {
            m_gui->BeginFrame();
            m_contentBrowser->Draw();
            m_viewport->Draw();
            m_gui->EndFrame();
        }

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
        // Panels before IGUI backend, IGUI before window/GL context.
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
}