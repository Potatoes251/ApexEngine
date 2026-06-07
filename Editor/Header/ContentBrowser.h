#ifndef CONTENT_BROWSER
#define CONTENT_BROWSER

// ============================================================
// ContentBrowser.h - Editor "Assets" window.
// Uses only IGUI - no ImGui headers required.
// ============================================================

#include "UI.h"

#include "Window.h"

#include "RHI.h"

#include "ResourceManager.h"

#include "ThumbnailRenderer.h"

#include <string>
#include <vector>
#include <filesystem>
#include <functional>

namespace Fs = std::filesystem;

namespace Apex::Editor
{
	// Asset type 
	enum class AssetType { Mesh, Texture, Sound, Level, Script, Hud, Material, Folder, Unknown };

	struct AssetEntry
	{
		Fs::path m_path;
		AssetType m_type = AssetType::Unknown;
		bool m_isFolder = false;
		uint32_t  m_thumbnailTextureID = 0;
	};

	// Called when the user double-clicks an asset to load it.
	using OnAssetLoadCallback = std::function<void(const AssetEntry&)>;

	class ContentBrowser
	{
	public:
		// gui      - IGUI reference owned by the Application.
		// rootPath - assets root directory (e.g. "Assets/").
		explicit ContentBrowser(Apex::UserInterface::IGUI& gui,
			Apex::Windowing::IWindow& window,
			const Fs::path& rootPath,
			Apex::Rendering::IRHI& rhi,
			Apex::Resources::ResourceManager& resourceManager);
		~ContentBrowser() = default;
		ContentBrowser(const ContentBrowser&) = delete;
		ContentBrowser& operator=(const ContentBrowser&) = delete;

		// Register a callback for when the user double-clicks a non-folder asset.
		void SetOnAssetLoad(OnAssetLoadCallback callback) { m_onLoad = std::move(callback); }

		// Returns true if the given screen-space position is inside the asset grid.
		bool ContainsGridPoint(float x, float y) const;

		// Call once per frame between gui.BeginFrame() and gui.EndFrame().
		void Draw();

		void PrepareThumbnails();

		ThumbnailRenderer& GetThumbnails();

		// Force a re-scan of the current folder (call after import / delete).
		void Refresh();

		// Returns true when the Content Browser panel (or its grid child) has focus.
		bool IsFocused() const { return m_isFocused; }

		// Returns the currently selected file path (empty if nothing selected).
		Fs::path GetSelectedPath() const { return m_selectedPath; }

		// Paste a previously copied file into the current folder.
		void PasteFile(const Fs::path& sourcePath);

		// Duplicate the currently selected file in-place (Ctrl+D).
		void DuplicateSelected();

		// Copy external files/folders into the currently open folder.
		// Called by the OS drop callback registered in Editor::InitPanels.
		void ImportFiles(std::vector<Fs::path> const& sources, const Fs::path& destDir = {});

		// The folder currently hovered in the tree, grid, or breadcrumb.
		// Empty when no folder is hovered. Reset every frame.
		// Used by the OS drop callback to route drops to a specific folder.
		Fs::path GetExternalDropDest(float mouseX, float mouseY) const;

	private:
		struct BreadcrumbRect
		{
			Fs::path m_path;
			float m_x, m_y, m_width, m_height;
		};

		// Layout helpers 
		void DrawToolbar();
		void DrawBreadcrumb();
		void DrawFolderTree(const Fs::path& dir);
		void DrawFolderTreeEntry(const Fs::directory_entry& entry);
		bool DrawTreeNode(const Fs::path& path, bool isRenaming);
		void HandleTreeNodeInput(const Fs::path& path, bool isRenaming);
		Apex::UserInterface::TreeNodeFlags BuildTreeNodeFlags(const Fs::path& path) const;
		void DrawAssetGrid();
		void DrawAddPopup();
		void DrawTile(const AssetEntry& entry);
		void DrawTileIcon(const AssetEntry& entry, LibMath::Vector2 pos);
		void DrawTileInteraction(const AssetEntry& entry, LibMath::Vector2 pos, float iconSize);
		void DrawTileTooltip(const AssetEntry& entry, uint32_t color);
		void DrawTileLabel(const AssetEntry& entry, LibMath::Vector2 pos);
		void DrawEntryContextMenu(const AssetEntry& entry);
		void DrawGridContextMenu();
		void DrawRenameInline(const AssetEntry& entry, LibMath::Vector2 pos);
		void DrawBreadcrumbPart(const Fs::path& accumulated, const std::string& label, int idx);
		void DrawTreeNodeRename();

		// Internal navigation / scanning 
		void NavigateTo(const Fs::path& dir);
		void ScanCurrentDirectory();
		void ScanSearch(const std::string& filter);
		void MoveEntry(const Fs::path& from, const Fs::path& toDirectory);
		void OnAddButtonClicked();
		void DeleteSelected();
		void DuplicateEntry(const AssetEntry& entry);
		void RevealInExplorer(const Fs::path& path);
		void BeginRename(const Fs::path& path);
		void CommitRename();
		void CreateFolder();
		void CreateAssetFile(const Fs::path& dir, const std::string& name, const std::string& ext, const std::string& content);

		// Static helpers
		static AssetType   ClassifyExtension(const std::string& ext);
		static const char* TypeLabel(AssetType type);
		static uint32_t    TypeColor(AssetType type);   // returns PackColor value
		static AssetEntry  ClassifyEntry(const Fs::directory_entry& entry);
		static const char* IconPathForType(AssetType type);
		static void		   PopulateThumbnailsForList(std::vector<AssetEntry>& entries, ThumbnailRenderer& thumbnails);
		static bool		   IsPathInsideProject(const Fs::path& projectRoot, const Fs::path& path);

		// State
		Apex::UserInterface::IGUI& m_gui;
		Apex::Windowing::IWindow& m_window;
		Apex::Resources::ResourceManager& m_resourceManager;
		ThumbnailRenderer					m_thumbnails;
		Fs::path							m_rootPath;
		Fs::path							m_currentPath;
		std::vector<AssetEntry>				m_entries;
		std::vector<AssetEntry>				m_searchEntries;
		char								m_lastSearch[256] = {};

		char   m_searchBuffer[256] = {};
		bool   m_pendingRefresh = false;
		int    m_tileSize = 120;
		bool   m_showTree = true;
		bool   m_isFocused = false;

		Fs::path             m_selectedPath;
		Fs::path             m_pendingNavigate;
		OnAssetLoadCallback  m_onLoad;

		// Rename state - active when m_renamingPath is non-empty
		Fs::path m_renamingPath;
		char     m_renameBuffer[256] = {};
		bool     m_renameFocusPending = false;

		// Asset grid child panel bounds - updated every frame in DrawAssetGrid()
		float m_gridX = 0.f, m_gridY = 0.f;
		float m_gridWidth = 0.f, m_gridHeight = 0.f;

		std::vector<BreadcrumbRect> m_breadCrumbRects;

		// Hovered drop destination for external OS drops.
		// m_hoveredDestThisFrame: set by hovered items each frame, cleared at start of Draw().
		// m_externalDropDest: sticky - only updated when something is hovered, persists
		//                     across frames so the OS drop callback can read it.
		Fs::path m_hoveredDestThisFrame;
		Fs::path m_externalDropDest;
	};
}
#endif