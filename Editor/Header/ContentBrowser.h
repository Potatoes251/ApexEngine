#ifndef CONTENT_BROWSER
#define CONTENT_BROWSER

// ============================================================
// ContentBrowser.h — Editor "Assets" window.
// Uses only IGUI — no ImGui headers required.
// ============================================================

#include "UI.h"
#include "Window.h"

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
		explicit ContentBrowser(Apex::UserInterface::IGUI& gui, 
								Apex::Windowing::IWindow& window, 
								const fs::path& rootPath);
		~ContentBrowser() = default;
		ContentBrowser(const ContentBrowser&) = delete;
		ContentBrowser& operator=(const ContentBrowser&) = delete;

		// Register a callback for when the user double-clicks a non-folder asset.
		void SetOnAssetLoad(OnAssetLoadCallback callback) { m_onLoad = std::move(callback); }

		// Returns true if the given screen-space position is inside the asset grid.
		bool ContainsGridPoint(float x, float y) const;

		// Call once per frame between gui.BeginFrame() and gui.EndFrame().
		void Draw();

		// Force a re-scan of the current folder (call after import / delete).
		void Refresh();

		// Copy external files/folders into the currently open folder.
		// Called by the OS drop callback registered in Editor::InitPanels.
		void ImportFiles(std::vector<fs::path> sources, const fs::path& destDir = {});

		// The folder currently hovered in the tree, grid, or breadcrumb.
		// Empty when no folder is hovered. Reset every frame.
		// Used by the OS drop callback to route drops to a specific folder.
		fs::path GetExternalDropDest(float mouseX, float mouseY) const;

	private:
		// Layout helpers 
		void DrawToolbar();
		void DrawBreadcrumb();
		void DrawFolderTree(const fs::path& dir);
		void DrawAssetGrid();
		void DrawAddPopup();
		void DrawTile(AssetEntry& entry);
		void DrawTileIcon(const AssetEntry& entry, LibMath::Vector2 pos);
		void DrawTileInteraction(AssetEntry& entry, LibMath::Vector2 pos, float iconSize);
		void DrawTileTooltip(const AssetEntry& entry, uint32_t color);
		void DrawTileLabel(const AssetEntry& entry, LibMath::Vector2 pos);
		void DrawEntryContextMenu(const AssetEntry& entry);
		void DrawGridContextMenu();
		void DrawRenameInline(AssetEntry& entry, LibMath::Vector2 pos);
		void DrawBreadcrumbPart(const fs::path& accumulated, const std::string& label, int idx);
		void DrawTreeNodeRename();

		// Internal navigation / scanning 
		void NavigateTo(const fs::path& dir);
		void ScanCurrentDirectory();
		void ScanSearch(const std::string& filter);
		void MoveEntry(const fs::path& from, const fs::path& toDirectory);
		void OnAddButtonClicked();
		void DeleteSelected();
		void DuplicateEntry(const AssetEntry& entry);
		void RevealInExplorer(const fs::path& path);
		void BeginRename(const fs::path& path);
		void CommitRename();
		void CreateFolder();

		// Static helpers
		static AssetType   ClassifyExtension(const std::string& ext);
		static const char* TypeLabel(AssetType type);
		static uint32_t    TypeColor(AssetType type);   // returns PackColor value
		static AssetEntry  ClassifyEntry(const fs::directory_entry& entry);

		// State
		Apex::UserInterface::IGUI& m_gui;
		Apex::Windowing::IWindow&  m_window;
		fs::path                   m_rootPath;
		fs::path                   m_currentPath;
		std::vector<AssetEntry>    m_entries;
		std::vector<AssetEntry>    m_searchEntries;
		char                       m_lastSearch[256] = {};

		char   m_searchBuffer[256] = {};
		int    m_tileSize = 120;
		bool   m_showTree = true;

		fs::path             m_selectedPath;
		fs::path             m_pendingNavigate;
		OnAssetLoadCallback  m_onLoad;

		// Rename state — active when m_renamingPath is non-empty
		fs::path m_renamingPath;
		char     m_renameBuffer[256] = {};
		bool     m_renameFocusPending = false;

		// Asset grid child panel bounds — updated every frame in DrawAssetGrid()
		float m_gridX = 0.f, m_gridY = 0.f;
		float m_gridWidth = 0.f, m_gridHeight = 0.f;

		struct BreadcrumbRect
		{
			fs::path m_path;
			float m_x, m_y, m_width, m_height;
		};
		std::vector<BreadcrumbRect> m_breadcrumbRects;

		// Hovered drop destination for external OS drops.
		// m_hoveredDestThisFrame: set by hovered items each frame, cleared at start of Draw().
		// m_externalDropDest: sticky — only updated when something is hovered, persists
		//                     across frames so the OS drop callback can read it.
		fs::path m_hoveredDestThisFrame;
		fs::path m_externalDropDest;
	};
}

#endif