#ifndef EDITOR
#define EDITOR

// ============================================================
// Editor.h - Top-level editor layer.
// Owns its own window, IGUI backend, and all editor panels.
// The game window stays alive independently - closing the
// editor window quits the process, closing the game window
// only stops the game tick.
// ============================================================

#include <filesystem>
#include "Application.h"
#include "ContentBrowser.h"
#include "Hierarchy.h"
#include "Viewport.h"
#include "TextureEditor.h"
#include "MeshViewer.h"
#include "SoundPlayer.h"
#include "HudEditor.h"
#include "ComponentsViewer.h"
#include "Console.h"
#include "MaterialEditor.h"

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

		// One editor frame: make editor context current, render all panels,
		void Render();

		void Update();

		void HandleEditorClosing();

		// Destroy panels and ImGui before the GL context tears down.
		void Shutdown();

		void Build(const std::vector<std::string>& scenePaths);

		// main.cpp uses this to drive the outer loop.
		Apex::Windowing::IWindow*	GetWindow() { return m_window.get(); }
		Apex::Application*			GetApp() { return m_app; }
		Apex::UserInterface::IGUI*	GetGUI() { return m_gui.get(); }
		void						SetEditorState(EditorState newState);
		EditorState					GetEditorState() const { return m_editorState; }

		// Shortcut handlers — each checks which panel is focused and acts accordingly.
		void CopySelected();
		void PasteClipboard();
		void DuplicateSelected();
			 
		void CleanClipboard();
			 
		void OnSceneChanged();

		void LoadCameraSettings();
		void SaveUserSettings();

	private:
		void BuildSetupOutputDir(const Fs::path& outputDir);
		void BuildCopyExecutables(const Fs::path& gameExeDir, const Fs::path& outputDir);
		void BuildCopyFonts(const Fs::path& outputDir);
		void BuildCopyEngineAssets(const Fs::path& outputDir);
		void BuildCopySceneFiles(const std::vector<std::string>& scenePaths, const Fs::path& outputDir);
		void BuildCollectDependencies(const std::vector<std::string>& scenePaths, std::unordered_set<std::string>& outAssets);
		void BuildCopyAssets(const std::unordered_set<std::string>& assets, const Fs::path& outputDir);
		void BuildWriteScenesJson(const std::vector<std::string>& scenePaths, const Fs::path& outputDir);

		static const char* TypeToString(AssetType type);

		void OpenTextureEditor(const std::filesystem::path& path);
		void OpenMeshViewer(const std::filesystem::path& path);
		void OpenSoundPlayer(const std::filesystem::path& path);
		void OpenHudEditor(const std::filesystem::path& path);
		void OpenMaterialEditor(const std::filesystem::path& path);

		void CollectAssetPaths(const std::string& scenePath,
			std::unordered_set<std::string>& outAssets,
			std::unordered_set<std::string>& outMats);
		void CollectLuaDependencies(const std::string& scriptPath,
			std::unordered_set<std::string>& out,
			std::unordered_set<std::string>& visited);
		void CollectHudDependencies(
			const std::string& scriptPath,
			std::unordered_set<std::string>& outHuds);
		void CollectAssetsFromMats(const std::string& matPath,
			std::unordered_set<std::string>& outAssets);

		Apex::Application* m_app = nullptr;
		Apex::Audio::IAudioEngine* m_audio;

		std::unique_ptr<Apex::Windowing::IWindow>	m_window;
		std::unique_ptr<Apex::UserInterface::IGUI>	m_gui;
		std::unique_ptr<ContentBrowser>				m_contentBrowser;
		std::unique_ptr<Hierarchy>					m_hierarchy;
		std::vector<std::unique_ptr<TextureEditor>> m_textureEditors;
		std::vector<std::unique_ptr<MeshViewer>>    m_meshViewers;
		std::vector<std::unique_ptr<SoundPlayer>>   m_soundPlayers;
		std::vector<std::unique_ptr<HudEditor>>     m_hudEditors;
		std::unique_ptr<MaterialEditor>				m_materialEditor;
		std::unique_ptr<Viewport>					m_viewport;
		std::unique_ptr<Rendering::Scene>			m_editScene;
		std::unique_ptr<ComponentsViewer>			m_componentsViewer;
		std::unique_ptr<Console>					m_console;

		EditorState	m_editorState = EditorState::Edit;

		// Clipboard for Ctrl+C / Ctrl+V / Ctrl+D shortcuts.
		// Object clipboard: set by Hierarchy/Viewport, consumed by Paste/Duplicate.
		size_t m_clipboardObjectId = 0;   // 0 == empty
		bool   m_hasClipboardObject = false;
		// File clipboard: set by ContentBrowser copy, consumed by Paste.
		std::filesystem::path m_clipboardFile;

		bool m_confirmClosePopup = false;
		bool m_editingUI = false;

		bool m_cameraSettingsLoaded = false;
	};
}

#endif