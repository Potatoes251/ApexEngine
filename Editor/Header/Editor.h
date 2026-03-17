#ifndef EDITOR
#define EDITOR

// ============================================================
// Editor.h - Top-level editor layer.
// Owns its own window, IGUI backend, and all editor panels.
// The game window stays alive independently - closing the
// editor window quits the process, closing the game window
// only stops the game tick.
// ============================================================

#include "Application.h"
#include "ContentBrowser.h"
#include "Hierarchy.h"
#include "Viewport.h"
#include "TextureEditor.h"

namespace Apex::Editor
{
	class Editor 
	{
	public:
		Editor() = default;
		~Editor() = default;

		Editor(const Editor&) = delete;
		Editor& operator=(const Editor&) = delete;

		// Creates the primary GLFW window and GL context.
		bool InitWindow();

		// Creates ImGui and all panels that need the live app.
		bool InitPanels(Apex::Application& app);

		// Call when the game window closes, before destroying Application.
		// Clears any panel references to the app so the reset is safe.
		void OnGameStopped();

		// One editor frame: make editor context current, render all panels,
		void Render();

		void Update();

		// Destroy panels and ImGui before the GL context tears down.
		void Shutdown();

		// main.cpp uses this to drive the outer loop.
		Apex::Windowing::IWindow& GetWindow() { return *m_window; }

	private:
		static const char* TypeToString(AssetType type);
		void OnExternalDrop(std::vector<std::filesystem::path> paths);

		// Opens a new TextureEditor for path, or focuses the existing one
		void OpenTextureEditor(const std::filesystem::path& path);

		Apex::Application* m_app = nullptr;
		std::unique_ptr<Apex::Audio::IAudioEngine>	m_audio;

		std::unique_ptr<Apex::Windowing::IWindow>	m_window;
		std::unique_ptr<Apex::UserInterface::IGUI>	m_gui;
		std::unique_ptr<ContentBrowser>				m_contentBrowser;
		std::unique_ptr<Hierarchy>					m_hierarchy;
		std::vector<std::unique_ptr<TextureEditor>> m_textureEditors;
		std::unique_ptr<Viewport>				    m_viewport;
	};
}

#endif