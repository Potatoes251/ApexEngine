#include "Editor.h"

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

        return true;
    }

    void Editor::OnGameStopped()
    {
        m_app = nullptr;
    }

    void Editor::Render()
    {
        if (!m_window) return;

        m_window->MakeContextCurrent();
        m_window->Clear();

        if (m_gui && m_contentBrowser)
        {
            m_gui->BeginFrame();
            m_contentBrowser->Draw();
            m_gui->EndFrame();
        }

        m_window->SwapBuffers();
    }

    void Editor::Shutdown()
    {
        // Panels before IGUI backend, IGUI before window/GL context.
        m_contentBrowser.reset();
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