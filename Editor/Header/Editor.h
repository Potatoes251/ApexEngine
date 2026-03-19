#ifndef EDITOR
#define EDITOR

// ============================================================
// Editor.h — Top-level editor layer.
// Owns its own window, IGUI backend, and all editor panels.
// The game window stays alive independently — closing the
// editor window quits the process, closing the game window
// only stops the game tick.
// ============================================================

#include "Application.h"
#include "ContentBrowser.h"
#include "Viewport.h"
#include "TextureEditor.h"

namespace Apex::Editor
{
	enum class EditorState
	{
		Edit,   // Editing the scene
		Play,   // Game is running
		Pause,  // Game is paused, can still inspect
		Step    // Advance one frame
	};

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
		Apex::Windowing::IWindow*	GetWindow() { return m_window.get(); }
		Apex::Application*			GetApp() { return m_app; }
		Apex::UserInterface::IGUI*	GetGUI() { return m_gui.get(); }
		void						SetEditorState(EditorState isTicking) { m_editorState = isTicking; }
		EditorState					GetEditorState() const { return m_editorState; }

	private:
		static const char* TypeToString(AssetType type);
		void OnExternalDrop(std::vector<std::filesystem::path> paths);

		// Opens a new TextureEditor for path, or focuses the existing one
		void OpenTextureEditor(const std::filesystem::path& path);

		Apex::Application* m_app = nullptr;
		Apex::Audio::IAudioEngine* m_audio;

		std::unique_ptr<Apex::Windowing::IWindow>  m_window;
		std::unique_ptr<Apex::UserInterface::IGUI> m_gui;
		std::unique_ptr<ContentBrowser>            m_contentBrowser;
		// One TextureEditor per open texture — grows on open, shrinks on close
		std::vector<std::unique_ptr<TextureEditor>> m_textureEditors;
		std::unique_ptr<Viewport>				   m_viewport;

		EditorState	m_editorState = EditorState::Edit;
	};
}

#endif