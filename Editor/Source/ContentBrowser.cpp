#include "ContentBrowser.h"

#include "EditorIcon.h"

#include "ThreadPool.h"
#include "MeshLoader.h"

#include "Log.h"

#include <iostream>
#include <fstream>

using namespace Apex::UserInterface;

namespace Apex::Editor
{
    AssetType ContentBrowser::ClassifyExtension(const std::string& ext)
    {
        if (ext == ".mesh")
            return AssetType::Mesh;
        if (ext == ".jpg" || ext == ".png" || ext == ".tga" || ext == ".hdr")
            return AssetType::Texture;
        if (ext == ".ogg" || ext == ".wav" || ext == ".mp3")
            return AssetType::Sound;
        if (ext == ".level")
            return AssetType::Level;
        if (ext == ".lua")
            return AssetType::Script;
        if (ext == ".hud")
            return AssetType::Hud;
        if (ext == ".mat")
            return AssetType::Material;
        return AssetType::Unknown;
    }

    const char* ContentBrowser::TypeLabel(AssetType type)
    {
        switch (type)
        {
        case AssetType::Mesh:    return "Mesh";
        case AssetType::Texture: return "Texture";
        case AssetType::Sound:   return "Sound";
        case AssetType::Level:   return "Level";
        case AssetType::Script:  return "Lua";
        case AssetType::Hud:     return "HUD";
        case AssetType::Folder:  return "Folder";
		case AssetType::Material:return "Material";
        default:                 return "[ ]";
        }
    }

    uint32_t ContentBrowser::TypeColor(AssetType type)
    {
        using Apex::UserInterface::PackColor;
        switch (type)
        {
        case AssetType::Mesh:    return PackColor(100, 180, 255);  // blue
        case AssetType::Material:return PackColor(10, 140, 20);    // dark green
        case AssetType::Texture: return PackColor(120, 230, 120);  // green
        case AssetType::Sound:   return PackColor(220, 120, 255);  // purple
        case AssetType::Level:   return PackColor(255, 100, 100);  // red
        case AssetType::Script:  return PackColor(255, 140, 60);   // orange
        case AssetType::Hud:     return PackColor(80, 220, 200);   // teal
        case AssetType::Folder:  return PackColor(255, 210, 100);  // gold
        default:                 return PackColor(160, 160, 160);  // grey
        }
    }

    AssetEntry ContentBrowser::ClassifyEntry(const Fs::directory_entry& entry)
    {
        AssetEntry assetEntry;
        assetEntry.m_path = entry.path();
        assetEntry.m_isFolder = entry.is_directory();
        if (assetEntry.m_isFolder)
        {
            assetEntry.m_type = AssetType::Folder;
            return assetEntry;
        }
        std::string ext = entry.path().extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        assetEntry.m_type = ClassifyExtension(ext);
        return assetEntry;
    }

    const char* ContentBrowser::IconPathForType(AssetType type)
    {
        switch (type)
        {
        case AssetType::Folder: return "ApexAssets/Icons/folder.png";
        case AssetType::Level:  return "ApexAssets/Icons/level.png";
        case AssetType::Script: return "ApexAssets/Icons/script.png";
        case AssetType::Sound:  return "ApexAssets/Icons/sound.png";
        case AssetType::Hud:    return "ApexAssets/Icons/hud.png";
        default:                return "";
        }

    }

    void ContentBrowser::PopulateThumbnailsForList(std::vector<AssetEntry>& entries, ThumbnailRenderer& thumbnails)
    {
        for (auto& entry : entries)
        {
            if (entry.m_thumbnailTextureID != 0)
                continue;

            if (entry.m_type == AssetType::Mesh)
            {
                uint32_t id = thumbnails.GetOrRender(entry.m_path);
                if (id != 0)
                    entry.m_thumbnailTextureID = id;
            }
            else if (entry.m_type == AssetType::Texture)
            {
                uint32_t id = EditorIcon::Get(entry.m_path.string());
                if (id != 0)
                    entry.m_thumbnailTextureID = id;
            }
            else if (entry.m_type == AssetType::Material)
            {
                uint32_t id = thumbnails.GetOrRenderMaterial(entry.m_path);
                if (id != 0)
                    entry.m_thumbnailTextureID = id;
            }
        }
    }

    ContentBrowser::ContentBrowser(Apex::UserInterface::IGUI& gui,
        Apex::Windowing::IWindow& window,
        const Fs::path& rootPath,
        Apex::Rendering::IRHI& rhi,
        Apex::Resources::ResourceManager& resourceManager)
        : m_gui(gui),
        m_window(window),
        m_rootPath(Fs::weakly_canonical(rootPath)),
        m_currentPath(Fs::weakly_canonical(rootPath)),
        m_resourceManager(resourceManager),
        m_thumbnails(rhi, resourceManager)
    {
        Refresh();
    }

    void ContentBrowser::Refresh()
    {
        ScanCurrentDirectory();
    }

    void ContentBrowser::NavigateTo(const Fs::path& dir)
    {
        //m_thumbnails.Clear();
        m_currentPath = Fs::weakly_canonical(dir);
        std::memset(m_searchBuffer, 0, sizeof(m_searchBuffer));
        ScanCurrentDirectory();
    }

    void ContentBrowser::ScanCurrentDirectory()
    {
        m_entries.clear();
        if (!Fs::exists(m_currentPath) || !Fs::is_directory(m_currentPath))
            return;

        std::vector<AssetEntry> folders, files;

        for (const auto& entry : Fs::directory_iterator(m_currentPath))
        {
            if (!entry.is_directory() && (entry.path().extension() == ".meta" || entry.path().extension() == ".obj"
                || entry.path().extension() == ".fbx" || entry.path().extension() == ".FBX" || entry.path().extension() == ".mtl"))
                continue;

            AssetEntry assetEntry = ClassifyEntry(entry);
            if (assetEntry.m_isFolder)
                folders.push_back(assetEntry);
            else
                files.push_back(assetEntry);
        }

        auto byName = [](const AssetEntry& a, const AssetEntry& b)
            { return a.m_path.filename() < b.m_path.filename(); };

        std::sort(folders.begin(), folders.end(), byName);
        std::sort(files.begin(), files.end(), byName);

        m_entries.insert(m_entries.end(), folders.begin(), folders.end());
        m_entries.insert(m_entries.end(), files.begin(), files.end());

        for (auto& entry : m_entries)
            entry.m_thumbnailTextureID = m_thumbnails.GetCached(entry.m_path);
    }

    void ContentBrowser::ScanSearch(const std::string& filter)
    {
        m_searchEntries.clear();

        const Fs::path& searchRoot = (m_currentPath == m_rootPath) ? m_rootPath : m_currentPath;

        for (const auto& entry : Fs::recursive_directory_iterator(searchRoot))
        {
            if (entry.is_directory()) continue;
            if (entry.path().extension() == ".meta" || entry.path().extension() == ".obj"
                || entry.path().extension() == ".fbx" || entry.path().extension() == ".FBX"
                || entry.path().extension() == ".mtl") continue;

            std::string name = entry.path().filename().string();
            std::transform(name.begin(), name.end(), name.begin(), ::tolower);
            if (name.find(filter) == std::string::npos) continue;

            AssetEntry assetEntry = ClassifyEntry(entry);
            m_searchEntries.push_back(assetEntry);
        }

        auto byName = [](const AssetEntry& a, const AssetEntry& b)
            { return a.m_path.filename() < b.m_path.filename(); };
        std::sort(m_searchEntries.begin(), m_searchEntries.end(), byName);

        for (auto& entry : m_searchEntries)
            entry.m_thumbnailTextureID = m_thumbnails.GetCached(entry.m_path);
    }

    void ContentBrowser::MoveEntry(const Fs::path& from, const Fs::path& toDirectory)
    {
        if (from == toDirectory || from.parent_path() == toDirectory) return;

        Fs::path destination = toDirectory / from.filename();

        std::error_code error;
        Fs::rename(from, destination, error);
        if (!error)
        {
            m_selectedPath.clear();
            Refresh();
        }
    }

    void ContentBrowser::OnAddButtonClicked()
    {
        m_gui.OpenPopup("##add_menu");
    }

    void ContentBrowser::DeleteSelected()
    {
        if (m_selectedPath.empty()) return;

        if (Fs::is_directory(m_selectedPath))
            Fs::remove_all(m_selectedPath);
        else
            Fs::remove(m_selectedPath);

        // If we just deleted the current directory or one of its ancestors,
        // navigate up to the closest existing parent.
        if (m_currentPath == m_selectedPath ||
            m_currentPath.string().rfind(m_selectedPath.string(), 0) == 0)
        {
            Fs::path parent = m_selectedPath.parent_path();
            if (!Fs::exists(parent) || parent == m_selectedPath)
                parent = m_rootPath;
            NavigateTo(parent);
        }

        m_selectedPath.clear();
        Refresh();
    }

    void ContentBrowser::DuplicateEntry(const AssetEntry& entry)
    {
        Fs::path src = entry.m_path;
        Fs::path stem = src.stem().string();
        Fs::path ext = src.extension();
        Fs::path dest = src.parent_path() / (stem.string() + "_copy" + ext.string());

        int n = 2;
        while (Fs::exists(dest))
            dest = src.parent_path() / (stem.string() + "_copy" + std::to_string(n++) + ext.string());

        Fs::copy_file(src, dest, Fs::copy_options::skip_existing);
        Refresh();
    }

    // Ctrl+D — duplicate the currently selected entry in-place.
    void ContentBrowser::DuplicateSelected()
    {
        if (m_selectedPath.empty() || Fs::is_directory(m_selectedPath))
            return;

        // Reuse DuplicateEntry by building a temporary AssetEntry.
        AssetEntry entry;
        entry.m_path = m_selectedPath;
        DuplicateEntry(entry);
    }

    // Ctrl+V — paste a file that was Ctrl+C'd into the current folder.
    // If the source is already in the current folder the result is a _copy.
    void ContentBrowser::PasteFile(const Fs::path& sourcePath)
    {
        if (sourcePath.empty() || !Fs::exists(sourcePath) || Fs::is_directory(sourcePath))
            return;

        Fs::path stem = sourcePath.stem();
        Fs::path ext = sourcePath.extension();
        Fs::path dest = m_currentPath / (stem.string() + ext.string());

        // If pasting into the same folder, or the name already exists, append _copy.
        if (Fs::exists(dest))
        {
            dest = m_currentPath / (stem.string() + "_copy" + ext.string());
            int n = 2;
            while (Fs::exists(dest))
                dest = m_currentPath / (stem.string() + "_copy" + std::to_string(n++) + ext.string());
        }

        Fs::copy_file(sourcePath, dest, Fs::copy_options::skip_existing);
        m_selectedPath = dest;
        Refresh();
    }

    void ContentBrowser::RevealInExplorer(const Fs::path& path)
    {
        std::system(("explorer /select,\"" + path.string() + "\"").c_str());
    }

    void ContentBrowser::BeginRename(const Fs::path& path)
    {
        m_renamingPath = path;
        m_renameFocusPending = true;

        // Pre-fill with stem only - extension is re-attached in CommitRename
        std::string stem = path.stem().string();
        std::strncpy(m_renameBuffer, stem.c_str(), sizeof(m_renameBuffer) - 1);
        m_renameBuffer[sizeof(m_renameBuffer) - 1] = '\0';
    }

    void ContentBrowser::CommitRename()
    {
        if (m_renamingPath.empty()) return;

        std::string newStem(m_renameBuffer);

        auto firstNonSpace = newStem.find_first_not_of(' ');
        if (firstNonSpace == std::string::npos)
        {
            // All spaces or empty - cancel silently
            m_renamingPath.clear();
            return;
        }
        newStem = newStem.substr(firstNonSpace);

        if (newStem != m_renamingPath.stem().string())
        {
            // Re-attach the original extension so the user can never change it
            Fs::path dest = m_renamingPath.parent_path() / (newStem + m_renamingPath.extension().string());
            Fs::rename(m_renamingPath, dest);

            if (m_selectedPath == m_renamingPath)
                m_selectedPath = dest;

            Refresh();
        }

        m_renamingPath.clear();
    }

    void ContentBrowser::CreateFolder()
    {
        // Find a unique name
        Fs::path target = m_currentPath / "New Folder";
        int n = 2;
        while (Fs::exists(target))
            target = m_currentPath / ("New Folder " + std::to_string(n++));

        Fs::create_directory(target);
        Refresh();

        // Immediately enter rename mode on the new folder
        BeginRename(target);
    }

    bool ContentBrowser::IsPathInsideProject(const Fs::path& projectRoot, const Fs::path& path)
    {
        std::error_code ec;

        Fs::path root = Fs::weakly_canonical(projectRoot, ec);
        Fs::path target = Fs::weakly_canonical(path, ec);

        if (ec) return false;

        auto relative = target.lexically_relative(root);

        return !relative.empty() && relative.native()[0] != '.';
    }

    void ContentBrowser::CreateAssetFile(const Fs::path& dir, const std::string& name, const std::string& ext, const std::string& content)
    {
        Fs::path path = dir / (name + ext);
        int n = 1;
        while (Fs::exists(path))
            path = dir / (name + std::to_string(n++) + ext);

        std::ofstream file(path);
        file << content;

        BeginRename(path);
    }

    void ContentBrowser::ImportFiles(std::vector<Fs::path> const& sources, const Fs::path& destDir)
    {
        const Fs::path& destination = destDir.empty() ? m_currentPath : destDir;
        bool anyChange = false;
        bool hasMeshImport = false;

        for (const auto& src : sources)
        {
            std::string ext = src.extension().string();
            bool isMesh = ext == ".obj" || ext == ".fbx" || ext == ".FBX";

            if (IsPathInsideProject(m_rootPath, src))
            {
                if (isMesh)
                {
                    Core::ThreadPool::Get().Enqueue(&MeshLoader::Import, src.string());
                    hasMeshImport = true;
                }
                continue;
            }

            std::error_code error;
            Fs::path target = destination / src.filename();

            if (Fs::is_directory(src))
                Fs::copy(src, target, Fs::copy_options::recursive, error);
            else
                Fs::copy_file(src, target, Fs::copy_options::none, error);

            if (error)
                std::cerr << "[ContentBrowser] Import failed for "
                << src << ": " << error.message() << "\n";
            else
            {
                if (isMesh)
                {
                    Core::ThreadPool::Get().Enqueue(&MeshLoader::Import, target.string());
                    hasMeshImport = true;
                }
                anyChange = true;
            }
        }

        if (anyChange)
        {
            m_externalDropDest.clear();
            Refresh();
        } 

        if (hasMeshImport)
            m_pendingRefresh = true;
    }

    bool ContentBrowser::ContainsGridPoint(float x, float y) const
    {
        return x >= m_gridX && x <= m_gridX + m_gridWidth &&
            y >= m_gridY && y <= m_gridY + m_gridHeight;
    }

    void ContentBrowser::Draw()
    {
        if (m_pendingRefresh)
        {
            Refresh();
            m_pendingRefresh = false;
        }

        m_gui.PushFont(FontID::Default);
        m_gui.BeginPanel("Content Browser");
        m_gui.PopFont();

        DrawToolbar();
        m_gui.Separator();
        DrawBreadcrumb();
        m_gui.Separator();
        DrawAddPopup();

        m_hoveredDestThisFrame.clear();

        if (m_showTree)
        {
            m_gui.BeginChildPanel("##cb_tree", 240.f, 0.f, true);
            DrawFolderTree(m_rootPath);
            m_gui.EndChildPanel();
            m_gui.SameLine();
        }

        m_gui.BeginChildPanel("##cb_grid", 0.f, 0.f, false);
        // Capture grid bounds for external drop hit-test
        LibMath::Vector2 gridPos = m_gui.GetPanelPos();
        LibMath::Vector2 gridSize = m_gui.GetPanelSize();
        m_gridX = gridPos[0];
        m_gridY = gridPos[1];
        m_gridWidth = gridSize[0];
        m_gridHeight = gridSize[1];
        m_isFocused = m_gui.IsPanelFocused();
        DrawAssetGrid();
        m_gui.EndChildPanel();

        if (!m_hoveredDestThisFrame.empty())
            m_externalDropDest = m_hoveredDestThisFrame;

        if (!m_selectedPath.empty() && m_renamingPath.empty())
        {
            if (m_gui.IsKeyPressed(UIKey::Delete))
                DeleteSelected();
        }

        if (m_gui.IsWindowHovered() && !m_gui.IsAnyItemHovered() &&
            (m_gui.IsMouseReleased(MouseButton::Left) || m_gui.IsMouseReleased(MouseButton::Right)))
        {
            m_selectedPath.clear();
            m_renamingPath.clear();
        }

        m_gui.EndPanel();
    }

    void ContentBrowser::PrepareThumbnails()
    {
        PopulateThumbnailsForList(m_entries, m_thumbnails);
        PopulateThumbnailsForList(m_searchEntries, m_thumbnails);
    }

    ThumbnailRenderer& ContentBrowser::GetThumbnails()
    {
        return m_thumbnails;
    }

    Fs::path ContentBrowser::GetExternalDropDest(float mouseX, float mouseY) const
    {
        for (const auto& rect : m_breadCrumbRects)
            if (mouseX >= rect.m_x && mouseX <= rect.m_x + rect.m_width &&
                mouseY >= rect.m_y && mouseY <= rect.m_y + rect.m_height)
                return rect.m_path;

        return m_externalDropDest;
    }

    void ContentBrowser::DrawToolbar()
    {
        m_gui.PushStyleVariable(StyleVariable::FramePadding, 8.f, 5.f);
        m_gui.PushFont(FontID::Default);

        if (m_gui.Button("+ Add"))
            OnAddButtonClicked();

        m_gui.SameLine(0.f, 16.f);
        m_gui.VerticalSeparator();
        m_gui.SameLine(0.f, 16.f);

        m_gui.SetNextItemWidth(220.f);
        m_gui.InputText("##cb_search", m_searchBuffer, sizeof(m_searchBuffer), "Search assets...");

        if (m_searchBuffer[0] != '\0')
        {
            m_gui.SameLine();
            if (m_gui.Button("x"))
                std::memset(m_searchBuffer, 0, sizeof(m_searchBuffer));
        }

        m_gui.SameLine(0.f, 16.f);
        m_gui.VerticalSeparator();
        m_gui.SameLine(0.f, 16.f);

        if (m_gui.Button("Refresh"))
            Refresh();

        m_gui.SameLine();
        if (m_gui.Button(m_showTree ? "Hide Folder Tab" : "Show Folder Tab"))
            m_showTree = !m_showTree;

        if (m_currentPath != m_rootPath)
        {
            m_gui.SameLine();
            if (m_gui.Button("<- Back"))
                NavigateTo(m_currentPath.parent_path());
        }

        m_gui.SameLine(0.f, 16.f);
        m_gui.VerticalSeparator();
        m_gui.SameLine(0.f, 16.f);

        m_gui.TextDisabled("Tile Size:");
        m_gui.SameLine();
        m_gui.SetNextItemWidth(120.f);
        m_gui.SliderInt("##tilesize", &m_tileSize, 80, 160);

        m_gui.PopFont();
        m_gui.PopStyleVariable();
    }

    void ContentBrowser::DrawBreadcrumb()
    {
        Fs::path displayPath = (m_searchBuffer[0] != '\0' && !m_selectedPath.empty())
            ? m_selectedPath.parent_path() : m_currentPath;

        Fs::path relative = Fs::relative(displayPath, m_rootPath.parent_path());
        Fs::path accumulated = m_rootPath.parent_path();

        m_breadCrumbRects.clear();

        m_gui.PushFont(FontID::Default);

        bool first = true;
        int  idx = 0;
        for (const auto& part : relative)
        {
            accumulated /= part;

            if (!first)
            {
                m_gui.SameLine(0.f, 2.f);
                m_gui.TextDisabled("/");
                m_gui.SameLine(0.f, 2.f);
            }

            first = false;

            DrawBreadcrumbPart(accumulated, part.string(), idx++);
        }

        m_gui.PopFont();
    }

    void ContentBrowser::DrawFolderTree(const Fs::path& dir)
    {
        if (!Fs::exists(dir)) return;

        m_gui.PushStyleVariable(StyleVariable::FramePadding, 4.f, 5.f);
        m_gui.PushStyleVariable(StyleVariable::ItemSpacing, 6.f, 6.f);
        m_gui.PushStyleVariable(StyleVariable::IndentSpacing, 18.f);
        m_gui.PushFont(FontID::Default);

        for (const auto& entry : Fs::directory_iterator(dir))
            if (entry.is_directory()) DrawFolderTreeEntry(entry);

        m_gui.PopFont();
        m_gui.PopStyleVariable(); // IndentSpacing
        m_gui.PopStyleVariable(); // ItemSpacing
        m_gui.PopStyleVariable(); // FramePadding
    }

    void ContentBrowser::DrawFolderTreeEntry(const Fs::directory_entry& entry)
    {
        const Fs::path& path = entry.path();
        bool isRenaming = (m_renamingPath == path);

        bool open = DrawTreeNode(path, isRenaming);
        HandleTreeNodeInput(path, isRenaming);

        AssetEntry assetEntry = ClassifyEntry(entry);
        DrawEntryContextMenu(assetEntry);

        if (open) 
        { 
            DrawFolderTree(path); 
            m_gui.EndTreeNode(); 
        }
    }

    bool ContentBrowser::DrawTreeNode(const Fs::path& path, bool isRenaming)
    {
        auto flags = BuildTreeNodeFlags(path);
        std::string label = isRenaming ? "##renaming_" + path.string() : path.filename().string();

        m_gui.PushColor(Apex::UserInterface::StyleColor::Text, { 1.f, 0.85f, 0.4f, 1.f });
        bool open = m_gui.BeginTreeNode(label, flags);
        m_gui.PopColor();
        return open;
    }

    void ContentBrowser::HandleTreeNodeInput(const Fs::path& path, bool isRenaming)
    {
        if (isRenaming) 
        { 
            DrawTreeNodeRename(); 
            return; 
        }

        if (m_gui.IsItemClicked())
        {
            m_selectedPath = path;
            NavigateTo(path);
        }
        if (m_gui.IsItemDoubleClicked()) BeginRename(path);
        if (m_gui.IsItemHovered() && m_gui.IsMouseReleased(MouseButton::Right))
            m_selectedPath = path;
        if (m_gui.IsItemHovered())
            m_hoveredDestThisFrame = path;

        if (m_gui.BeginDropTarget())
        {
            const void* data = m_gui.AcceptDragPayload("ASSET_PATH");
            if (data)
                MoveEntry(Fs::path(static_cast<const char*>(data)), path);
            m_hoveredDestThisFrame = path;
            m_gui.EndDropTarget();
        }
    }

    Apex::UserInterface::TreeNodeFlags ContentBrowser::BuildTreeNodeFlags(const Fs::path& path) const
    {
        using Apex::UserInterface::TreeNodeFlags;
        bool hasSubDirs = false;
        for (const auto& sub : Fs::directory_iterator(path))
        {
            if (sub.is_directory())
            {
                hasSubDirs = true;
                break;
            }
        }

        TreeNodeFlags flags = TreeNodeFlags::OpenOnArrow | TreeNodeFlags::SpanAvailWidth;
        if (!hasSubDirs)         flags = flags | TreeNodeFlags::Leaf;
        if (path == m_currentPath) flags = flags | TreeNodeFlags::Selected;

        return flags;
    }

    void ContentBrowser::DrawAssetGrid()
    {
        bool searching = m_searchBuffer[0] != '\0';

        if (searching)
        {
            std::string filter = m_searchBuffer;
            std::transform(filter.begin(), filter.end(), filter.begin(), ::tolower);
            if (std::strncmp(m_searchBuffer, m_lastSearch, sizeof(m_searchBuffer)) != 0)
            {
                ScanSearch(filter);
                std::strncpy(m_lastSearch, m_searchBuffer, sizeof(m_lastSearch));
            }
        }
        else if (m_lastSearch[0] != '\0')
        {
            m_searchEntries.clear();
            std::memset(m_lastSearch, 0, sizeof(m_lastSearch));
        }

        std::vector<AssetEntry>& source = searching ? m_searchEntries : m_entries;

        LibMath::Vector2 availableSize = m_gui.GetAvailableSize();
        int columns = std::max(1, static_cast<int>(availableSize[0] / (m_tileSize + 16.f)));
        int col = 0;

        m_pendingNavigate.clear();

        for (auto& entry : const_cast<std::vector<AssetEntry>&>(source))
        {
            if (col > 0 && col % columns != 0)
                m_gui.SameLine();
            DrawTile(entry);
            col++;
        }

        if (!m_pendingNavigate.empty())
            NavigateTo(m_pendingNavigate);

        // Right-click on empty grid background
        DrawGridContextMenu();

        // F2 - rename selected entry
        if (!m_selectedPath.empty() && m_gui.IsKeyPressed(UIKey::F2))
            BeginRename(m_selectedPath);

    }

    void ContentBrowser::DrawAddPopup()
    {
        if (!m_gui.BeginPopup("##add_menu")) return;

        m_gui.PushStyleVariable(StyleVariable::WindowPadding, 10.f, 8.f);
        m_gui.PushStyleVariable(StyleVariable::ItemSpacing, 10.f, 8.f);
        m_gui.PushFont(FontID::Default);

        if (m_gui.MenuItem("New Folder"))
            CreateFolder();

        if (m_gui.MenuItem("Import Files..."))
        {
            auto paths = m_window.OpenFileDialog(true);
            if (!paths.empty())
                ImportFiles(paths);
        }

        if (m_gui.MenuItem("Import Folder..."))
        {
            auto folder = m_window.OpenFolderDialog();
            if (!folder.empty())
                ImportFiles({ folder });
        }

        m_gui.PopFont();
        m_gui.PopStyleVariable();
        m_gui.PopStyleVariable();

        m_gui.Separator();
        m_gui.TextDisabled("Create");
        m_gui.Separator();

        if (m_gui.MenuItem("New HUD"))
        {
            CreateAssetFile(m_currentPath, "NewHUD", ".hud", "{\n  \"widgets\": []\n}\n");
            Refresh();
        }

        if (m_gui.MenuItem("New Material"))
        {
            CreateAssetFile(m_currentPath, "NewMaterial", ".mat", "{\n}");
            Refresh();
        }

        if (m_gui.MenuItem("New Level"))
        {
            CreateAssetFile(m_currentPath, "NewLevel", ".level", "{\n  \"materials\": [],\n  \"objects\": []\n}\n");
            Refresh();
        }

        if (m_gui.MenuItem("New Script"))
        {
            CreateAssetFile(m_currentPath, "NewScript", ".lua", "-- New Script\n");
            Refresh();
        }

        m_gui.EndPopup();
    }

    void ContentBrowser::DrawTile(const AssetEntry& entry)
    {
        using Apex::UserInterface::PackColor;
        constexpr float LABEL_HEIGHT = 32.f;
        float totalHeight = m_tileSize + LABEL_HEIGHT;

        m_gui.PushID(entry.m_path.string());

        bool isSelected = (!m_selectedPath.empty() && m_selectedPath == entry.m_path);

        m_gui.BeginChildPanel("##tile", m_tileSize, totalHeight, false);

        LibMath::Vector2 pos = m_gui.GetCursorPos();

        DrawTileIcon(entry, pos);
        DrawTileInteraction(entry, pos, m_tileSize);

        if (isSelected)
        {
            IDrawList* drawList = m_gui.GetDrawList();
            LibMath::Vector2 thumbnailMin(pos[0], pos[1]);
            LibMath::Vector2 thumbnailMax(pos[0] + m_tileSize, pos[1] + totalHeight);
            drawList->DrawRectFilled(thumbnailMin, thumbnailMax, 0x336699FF, 0.f);
            drawList->DrawRect(thumbnailMin, thumbnailMax, 0xFF6699FF, 0.f, 2.f);
        }

        if (m_renamingPath == entry.m_path)
            DrawRenameInline(entry, pos);
        else
            DrawTileLabel(entry, pos);

        m_gui.EndChildPanel();

        m_gui.PopID();
    }

    void ContentBrowser::DrawTileIcon(const AssetEntry& entry, LibMath::Vector2 pos)
    {
        using Apex::UserInterface::PackColor;
        IDrawList* drawList = m_gui.GetDrawList();
        float      iconSize = m_tileSize;
        uint32_t   color = TypeColor(entry.m_type);

        LibMath::Vector2 thumbnailMin(pos[0], pos[1]);
        LibMath::Vector2 thumbnailMax(pos[0] + iconSize, pos[1] + iconSize);

        bool hasThumb = (entry.m_thumbnailTextureID != 0);
        if (hasThumb)
        {
            m_gui.SetCursorPos(thumbnailMin);
            m_gui.DrawImageTinted(entry.m_thumbnailTextureID,
                LibMath::Vector2(iconSize, iconSize), { 1.f,1.f,1.f,1.f }, true);
        }
        else
        {
            uint32_t icon = EditorIcon::Get(IconPathForType(entry.m_type));
            if (icon != 0)
            {
                drawList->DrawRectFilled(thumbnailMin, thumbnailMax, PackColor(30, 30, 35), 6.f);
                m_gui.SetCursorPos(LibMath::Vector2(thumbnailMin[0] + iconSize * 0.15f, thumbnailMin[1] + iconSize * 0.15f));
                m_gui.DrawImageTinted(icon,
                    LibMath::Vector2(iconSize * 0.7f, iconSize * 0.7f), { 1.f,1.f,1.f,1.f }, true);
            }
            else
            {
                drawList->DrawRectFilled(thumbnailMin, thumbnailMax, PackColor(30, 30, 35), 6.f);
                LibMath::Vector2 lblSize = drawList->CalculateTextSize(TypeLabel(entry.m_type));
                drawList->DrawText(
                    LibMath::Vector2(thumbnailMin[0] + (iconSize - lblSize[0]) * 0.5f,
                        thumbnailMin[1] + (iconSize - lblSize[1]) * 0.5f), color, TypeLabel(entry.m_type));
            }
        }

        constexpr float BADGE_H = 18.f;
        constexpr float BADGE_PAD = 5.f;
        const char* badgeTxt = TypeLabel(entry.m_type);
        LibMath::Vector2 badgeTxtSz = drawList->CalculateTextSize(badgeTxt);
        LibMath::Vector2 badgeMin(thumbnailMin[0] + 4.f, thumbnailMin[1] + 4.f);
        LibMath::Vector2 badgeMax(badgeMin[0] + badgeTxtSz[0] + BADGE_PAD * 2.f, badgeMin[1] + BADGE_H);
        drawList->DrawRectFilled(badgeMin, badgeMax, color, 4.f);
        drawList->DrawText(LibMath::Vector2(badgeMin[0] + BADGE_PAD,
            badgeMin[1] + (BADGE_H - badgeTxtSz[1]) * 0.5f),
            0xFF000000, badgeTxt);
    }


    void ContentBrowser::DrawTileInteraction(const AssetEntry& entry, LibMath::Vector2 pos, float iconSize)
    {
        float pad = 6.f;

        m_gui.SetCursorPos(LibMath::Vector2(pos[0] + pad, pos[1] + pad));
        m_gui.InvisibleButton("##btn", LibMath::Vector2(iconSize, iconSize));

        if (m_gui.IsItemClicked() ||
            (m_gui.IsItemHovered() && m_gui.IsMouseReleased(MouseButton::Right)))
            m_selectedPath = entry.m_path;

        if (m_gui.IsItemDoubleClicked())
        {
            if (entry.m_isFolder)
                m_pendingNavigate = entry.m_path;
            else if (m_onLoad)
            {
                m_onLoad(entry);
                m_selectedPath.clear();
            }
        }

        if (m_gui.BeginDragSource())
        {
            std::string pathStr = entry.m_path.string();
            m_gui.SetDragPayload("ASSET_PATH", pathStr.c_str(), pathStr.size() + 1);
            m_gui.TextDisabled(entry.m_path.filename().string());
            m_gui.EndDragSource();
        }

        if (entry.m_isFolder && m_gui.BeginDropTarget())
        {
            const void* data = m_gui.AcceptDragPayload("ASSET_PATH");
            if (data)
                MoveEntry(Fs::path(static_cast<const char*>(data)), entry.m_path);
            m_hoveredDestThisFrame = entry.m_path;
            m_gui.EndDropTarget();
        }
        else if (entry.m_isFolder && m_gui.IsItemHovered())
            m_hoveredDestThisFrame = entry.m_path;

        DrawEntryContextMenu(entry);

        if (m_gui.IsItemHovered())
            DrawTileTooltip(entry, TypeColor(entry.m_type));
    }

    void ContentBrowser::DrawTileTooltip(const AssetEntry& entry, uint32_t color)
    {
        // Shows a tooltip with the asset type badge, full filename, and file size.
        m_gui.BeginTooltip();

        // Unpack color to normalized floats for TextColored
        float r = ((color >> 0) & 0xFF) / 255.f;
        float g = ((color >> 8) & 0xFF) / 255.f;
        float b = ((color >> 16) & 0xFF) / 255.f;
        m_gui.TextColored({ r, g, b, 1.f }, TypeLabel(entry.m_type));
        m_gui.SameLine();
        m_gui.Text(entry.m_path.filename().string());

        if (!entry.m_isFolder)
        {
            std::error_code error;
            auto bytes = Fs::file_size(entry.m_path, error);
            if (!error)
                m_gui.TextDisabled(std::to_string(bytes / 1024) + " KB");
        }

        m_gui.EndTooltip();
    }

    void ContentBrowser::DrawTileLabel(const AssetEntry& entry, LibMath::Vector2 pos)
    {
        IDrawList* drawList = m_gui.GetDrawList();
        std::string name = entry.m_isFolder
            ? entry.m_path.filename().string()
            : entry.m_path.stem().string();
        if (name.size() > 14) name = name.substr(0, 13) + "~";

        float labelY = pos[1] + m_tileSize;
        LibMath::Vector2 bgMin(pos[0], labelY);
        LibMath::Vector2 bgMax(pos[0] + m_tileSize, labelY + 32.f);
        drawList->DrawRectFilled(bgMin, bgMax, 0xFF1E1E1E);

        LibMath::Vector2 textSize = drawList->CalculateTextSize(name.c_str());
        float centeredX = pos[0] + (m_tileSize - textSize[0]) * 0.5f;
        float centeredY = labelY + (32.f - textSize[1]) * 0.5f;

        m_gui.SetCursorPos(LibMath::Vector2(centeredX, centeredY));
        m_gui.InvisibleButton("##label", textSize);
        if (m_gui.IsItemDoubleClicked()) BeginRename(entry.m_path);

        drawList->DrawText(LibMath::Vector2(centeredX, centeredY), 0xFFDDDDDD, name);
    }

    void ContentBrowser::DrawEntryContextMenu(const AssetEntry& entry)
    {
        if (!m_gui.BeginContextMenu("##ctx_" + entry.m_path.string())) return;

        m_gui.PushStyleVariable(StyleVariable::WindowPadding, 10.f, 8.f);
        m_gui.PushStyleVariable(StyleVariable::ItemSpacing, 10.f, 8.f);

        m_gui.PushFont(FontID::Default);

        m_gui.TextDisabled(entry.m_path.filename().string());
        m_gui.Separator();

        if (entry.m_isFolder)
            if (m_gui.MenuItem("Open")) NavigateTo(entry.m_path);

        else if (m_gui.MenuItem("Load") && m_onLoad)
            m_onLoad(entry);

        m_gui.Separator();

        if (m_gui.MenuItem("Rename"))                          BeginRename(entry.m_path);
        if (!entry.m_isFolder && m_gui.MenuItem("Duplicate"))  DuplicateEntry(entry);
        if (m_gui.MenuItem("Copy Path"))                       m_gui.SetClipboardText(entry.m_path.string());

        m_gui.Separator();

        if (m_gui.MenuItem("Reveal in Explorer"))  RevealInExplorer(entry.m_path);

        m_gui.Separator();

        if (m_gui.MenuItem("Delete")) 
        { 
            m_selectedPath = entry.m_path; 
            DeleteSelected(); 
        }

        m_gui.PopFont();
        m_gui.PopStyleVariable();
        m_gui.PopStyleVariable();

        m_gui.EndContextMenu();
    }

    void ContentBrowser::DrawGridContextMenu()
    {
        if (!m_gui.BeginGridContextMenu()) return;

        m_gui.PushStyleVariable(StyleVariable::WindowPadding, 10.f, 8.f);
        m_gui.PushStyleVariable(StyleVariable::ItemSpacing, 10.f, 8.f);
        m_gui.PushFont(FontID::Default);

        m_gui.TextDisabled("Actions");
        m_gui.Separator();

        if (m_gui.MenuItem("New Folder"))
            CreateFolder();

        m_gui.Separator();

        if (m_gui.MenuItem("Import Files..."))
        {
            auto paths = m_window.OpenFileDialog(true);
            if (!paths.empty())
                ImportFiles(paths);
        }

        if (m_gui.MenuItem("Import Folder..."))
        {
            auto folder = m_window.OpenFolderDialog();
            if (!folder.empty())
                ImportFiles({ folder });
        }

        m_gui.Separator();
        m_gui.TextDisabled("Create");
        m_gui.Separator();

        if (m_gui.MenuItem("New HUD"))
        {
            CreateAssetFile(m_currentPath, "NewHUD", ".hud", "{\n  \"widgets\": []\n}\n");
            Refresh();
        }

        if (m_gui.MenuItem("New Material"))
        {
            CreateAssetFile(m_currentPath, "NewMaterial", ".mat", "{\n}");
            Refresh();
        }

        if (m_gui.MenuItem("New Level"))
        {
            CreateAssetFile(m_currentPath, "NewLevel", ".level", "{\n  \"materials\": [],\n  \"objects\": []\n}\n");
            Refresh();
        }

        if (m_gui.MenuItem("New Script"))
        {
            CreateAssetFile(m_currentPath, "NewScript", ".lua", "-- New Script\n");
            Refresh();
        }

        m_gui.Separator();
        if (m_gui.MenuItem("Refresh"))
            Refresh();

        if (m_gui.MenuItem("Reveal in Explorer"))
            std::system(("explorer \"" + m_currentPath.string() + "\"").c_str());

        m_gui.PopFont();
        m_gui.PopStyleVariable();
        m_gui.PopStyleVariable();

        m_gui.EndContextMenu();
    }

    void ContentBrowser::DrawRenameInline(const AssetEntry& entry, LibMath::Vector2 pos)
    {
        if (m_renamingPath != entry.m_path) return;

        if (m_renameFocusPending)
        {
            m_gui.SetKeyboardFocusHere();
            m_renameFocusPending = false;
        }

        m_gui.SetCursorPos(LibMath::Vector2(pos[0], pos[1] + m_tileSize + 2.f));
        m_gui.SetNextItemWidth(m_tileSize);
        m_gui.InputText("##rename", m_renameBuffer, sizeof(m_renameBuffer));

        if (m_gui.IsKeyPressed(UIKey::Enter))
            CommitRename();
        else if (m_gui.IsKeyPressed(UIKey::Escape))
            m_renamingPath.clear();
    }

    void ContentBrowser::DrawBreadcrumbPart(const Fs::path& accumulated, const std::string& label, int idx)
    {
        Apex::UserInterface::IDrawList* drawList = m_gui.GetDrawList();
        constexpr uint32_t COL_NORMAL = 0xFFCCCCCC;
        constexpr uint32_t COL_HOVERED = 0xFFFFFFFF;
        constexpr float    PAD_X = 6.f;
        constexpr float    PAD_Y = 3.f;

        LibMath::Vector2 textSize = drawList->CalculateTextSize(label);
        float buttonWidth = textSize[0] + PAD_X * 2.f;
        float buttonHeight = textSize[1] + PAD_Y * 2.f;

        LibMath::Vector2 buttonPos = m_gui.GetCursorPos();

        if (Fs::is_directory(accumulated))
            m_breadCrumbRects.push_back({ accumulated, buttonPos[0], buttonPos[1], buttonWidth, buttonHeight });

        m_gui.PushID(idx);
        bool clicked = m_gui.InvisibleButton("##bc", LibMath::Vector2(buttonWidth, buttonHeight));
        bool hovered = m_gui.IsItemHovered();
        m_gui.PopID();

        drawList->DrawText(LibMath::Vector2(buttonPos[0] + PAD_X, buttonPos[1]),
            hovered ? COL_HOVERED : COL_NORMAL, label);

        if (!Fs::is_directory(accumulated)) return;

        if (clicked) NavigateTo(accumulated);
        if (hovered) m_hoveredDestThisFrame = accumulated;

        if (m_gui.BeginDropTarget())
        {
            const void* data = m_gui.AcceptDragPayload("ASSET_PATH");
            if (data)
                MoveEntry(Fs::path(static_cast<const char*>(data)), accumulated);
            m_hoveredDestThisFrame = accumulated;
            m_gui.EndDropTarget();
        }
    }

    void ContentBrowser::DrawTreeNodeRename()
    {
        if (m_renameFocusPending)
        {
            m_gui.SetKeyboardFocusHere();
            m_renameFocusPending = false;
        }

        float rowHeight = m_gui.GetItemRectSize()[1];
        m_gui.SameLine(0.f, 0.f);
        m_gui.SetNextItemWidth(-1.f);
        m_gui.PushStyleVariable(StyleVariable::FramePadding, 0.f, (rowHeight - m_gui.GetTextLineHeight()) * 0.5f);
        m_gui.InputText("##tree_rename", m_renameBuffer, sizeof(m_renameBuffer));
        m_gui.PopStyleVariable();

        if (m_gui.IsKeyPressed(UIKey::Enter))  CommitRename();
        else if (m_gui.IsKeyPressed(UIKey::Escape)) m_renamingPath.clear();
    }

}