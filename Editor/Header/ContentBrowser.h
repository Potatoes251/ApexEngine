#ifndef CONTENT_BROWSER
#define CONTENT_BROWSER

// ============================================================
// ContentBrowser.h — Editor "Assets" window.
// Uses only IGUI — no ImGui headers required.
// ============================================================

#include "UI.h"

#include <string>
#include <vector>
#include <filesystem>
#include <functional>

namespace fs = std::filesystem;

namespace Apex::Editor
{
	// Asset type 
	enum class AssetType { Mesh, Texture, Sound, Level, Folder, Unknown };

	struct AssetEntry
	{
		fs::path m_path;
		AssetType m_type = AssetType::Unknown;
		bool m_isFolder = false;
	};

	// Called when the user double-clicks an asset to load it.
	using OnAssetLoadCallback = std::function<void(const AssetEntry&)>;

	class ContentBrowser
	{
	public:
		// gui      — IGUI reference owned by the Application.
		// rootPath — assets root directory (e.g. "Assets/").
		explicit ContentBrowser(Apex::UserInterface::IGUI& gui, const fs::path& rootPath);
		~ContentBrowser() = default;
		ContentBrowser(const ContentBrowser&) = delete;
		ContentBrowser& operator=(const ContentBrowser&) = delete;

		// Register a callback for when the user double-clicks a non-folder asset.
		void SetOnAssetLoad(OnAssetLoadCallback callback) { m_onLoad = std::move(callback); }

		// Call once per frame between gui.BeginFrame() and gui.EndFrame().
		void Draw();

		// Force a re-scan of the current folder (call after import / delete).
		void Refresh();

	private:
		// Layout helpers 
		void DrawToolbar();
		void DrawBreadcrumb();
		void DrawFolderTree(const fs::path& dir);
		void DrawAssetGrid();
		void DrawTile(AssetEntry& entry);
		void DrawTileIcon(const AssetEntry& entry, LibMath::Vector2 pos);
		void DrawTileInteraction(AssetEntry& entry, LibMath::Vector2 pos, float iconSize);
		void DrawTileTooltip(const AssetEntry& entry, uint32_t color);
		void DrawTileLabel(const AssetEntry& entry, LibMath::Vector2 pos);
		void DrawContextMenu(const AssetEntry& entry);

		// Internal navigation / scanning 
		void NavigateTo(const fs::path& dir);
		void ScanCurrentDirectory();

		// Static helpers
		static AssetType   ClassifyExtension(const std::string& ext);
		static const char* TypeLabel(AssetType type);
		static uint32_t    TypeColor(AssetType type);   // returns PackColor value

		// State
		Apex::UserInterface::IGUI& m_gui;
		fs::path                   m_rootPath;
		fs::path                   m_currentPath;
		std::vector<AssetEntry>    m_entries;

		char   m_searchBuffer[256] = {};
		float  m_tileSize = 80.f;
		bool   m_showTree = true;

		AssetEntry* m_selected = nullptr;
		OnAssetLoadCallback  m_onLoad;
	};
}

#endif