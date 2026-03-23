#include "ContentBrowser.h"

#include <iostream>

using namespace Apex::UserInterface;

namespace Apex::Editor
{
    AssetType ContentBrowser::ClassifyExtension(const std::string& ext)
    {
        if (ext == ".obj" || ext == ".fbx")                  
            return AssetType::Mesh;
        if (ext == ".jpg" || ext == ".png" || ext == ".tga" || ext == ".hdr") 
            return AssetType::Texture;
        if (ext == ".ogg" || ext == ".wav" || ext == ".mp3") 
            return AssetType::Sound;
        if (ext == ".level")                                 
            return AssetType::Level;
        return AssetType::Unknown;
    }

    const char* ContentBrowser::TypeLabel(AssetType type)
    {
        switch (type)
        {
        case AssetType::Mesh:    return "[M]";
        case AssetType::Texture: return "[T]";
        case AssetType::Sound:   return "[S]";
        case AssetType::Level:   return "[L]";
        case AssetType::Folder:  return "[/]";
        default:                 return "[ ]";
        }
    }

    uint32_t ContentBrowser::TypeColor(AssetType type)
    {
        using Apex::UserInterface::PackColor;
        switch (type)
        {
        case AssetType::Mesh:    return PackColor(100, 180, 255);  // blue
        case AssetType::Texture: return PackColor(120, 230, 120);  // green
        case AssetType::Sound:   return PackColor(220, 120, 255);  // purple
        case AssetType::Level:   return PackColor(255, 100, 100);  // red
        case AssetType::Folder:  return PackColor(255, 210, 100);  // gold
        default:                 return PackColor(160, 160, 160);  // grey
        }
    }

    AssetEntry ContentBrowser::ClassifyEntry(const fs::directory_entry& entry)
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

    ContentBrowser::ContentBrowser(Apex::UserInterface::IGUI& gui,
                                   Apex::Windowing::IWindow& window,
                                   const fs::path& rootPath)
        : m_gui(gui),
        m_window(window),
        m_rootPath(fs::weakly_canonical(rootPath)),
        m_currentPath(fs::weakly_canonical(rootPath))
    {
        Refresh();
    }

    void ContentBrowser::Refresh() 
    { 
        ScanCurrentDirectory(); 
    }

    void ContentBrowser::NavigateTo(const fs::path& dir)
    {
        m_currentPath = fs::weakly_canonical(dir);
        std::memset(m_searchBuffer, 0, sizeof(m_searchBuffer));
        ScanCurrentDirectory();
    }

    void ContentBrowser::ScanCurrentDirectory()
    {
        m_entries.clear();
        if (!fs::exists(m_currentPath) || !fs::is_directory(m_currentPath))
            return;

        std::vector<AssetEntry> folders, files;

        for (const auto& entry : fs::directory_iterator(m_currentPath))
        {
            if (!entry.is_directory() && (entry.path().extension() == ".meta" || entry.path().extension() == ".mesh"))
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
    }

    void ContentBrowser::ScanSearch(const std::string& filter)
    {
        m_searchEntries.clear();

        const fs::path& searchRoot = (m_currentPath == m_rootPath) ? m_rootPath : m_currentPath;

        for (const auto& entry : fs::recursive_directory_iterator(searchRoot))
        {
            if (entry.is_directory()) continue;
            if (entry.path().extension() == ".meta") continue;

            std::string name = entry.path().filename().string();
            std::string low = name;
            std::transform(low.begin(), low.end(), low.begin(), ::tolower);
            if (low.find(filter) == std::string::npos) continue;

            AssetEntry assetEntry = ClassifyEntry(entry);
            m_searchEntries.push_back(assetEntry);
        }

        auto byName = [](const AssetEntry& a, const AssetEntry& b)
            { return a.m_path.filename() < b.m_path.filename(); };
        std::sort(m_searchEntries.begin(), m_searchEntries.end(), byName);
    }

    void ContentBrowser::MoveEntry(const fs::path& from, const fs::path& toDirectory)
    {
        if (from == toDirectory || from.parent_path() == toDirectory) return;

        fs::path destination = toDirectory / from.filename();
        
        std::error_code error;
        fs::rename(from, destination, error);
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

        std::error_code error;
        if (fs::is_directory(m_selectedPath))
            fs::remove_all(m_selectedPath, error);
        else
            fs::remove(m_selectedPath, error);

        // If we just deleted the current directory or one of its ancestors,
        // navigate up to the closest existing parent.
        if (m_currentPath == m_selectedPath ||
            m_currentPath.string().rfind(m_selectedPath.string(), 0) == 0)
        {
            fs::path parent = m_selectedPath.parent_path();
            if (!fs::exists(parent) || parent == m_selectedPath)
                parent = m_rootPath;
            NavigateTo(parent);
        }

        m_selectedPath.clear();
        Refresh();
    }

    void ContentBrowser::DuplicateEntry(const AssetEntry& entry)
    {
        fs::path src = entry.m_path;
        fs::path stem = src.stem().string();
        fs::path ext = src.extension();
        fs::path dest = src.parent_path() / (stem.string() + "_copy" + ext.string());

        int n = 2;
        while (fs::exists(dest))
            dest = src.parent_path() / (stem.string() + "_copy" + std::to_string(n++) + ext.string());

        std::error_code error;
        fs::copy_file(src, dest, fs::copy_options::skip_existing, error);
        Refresh();
    }

    void ContentBrowser::RevealInExplorer(const fs::path& path)
    {
        std::system(("explorer /select,\"" + path.string() + "\"").c_str());
    }

    void ContentBrowser::BeginRename(const fs::path& path)
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
            fs::path dest = m_renamingPath.parent_path() / (newStem + m_renamingPath.extension().string());
            std::error_code error;
            fs::rename(m_renamingPath, dest, error);

            if (m_selectedPath == m_renamingPath)
                m_selectedPath = dest;

            Refresh();
        }

        m_renamingPath.clear();
    }

    void ContentBrowser::CreateFolder()
    {
        // Find a unique name
        fs::path base = m_currentPath / "New Folder";
        fs::path target = base;
        int n = 2;
        while (fs::exists(target))
            target = m_currentPath / ("New Folder " + std::to_string(n++));

        std::error_code error;
        fs::create_directory(target, error);
        Refresh();

        // Immediately enter rename mode on the new folder
        BeginRename(target);
    }

    void ContentBrowser::ImportFiles(std::vector<fs::path> sources, const fs::path& destDir)
    {
        const fs::path& destination = destDir.empty() ? m_currentPath : destDir;
        bool anyChange = false;

        for (const auto& src : sources)
        {
            std::error_code error;
            fs::path target = destination / src.filename();

            if (fs::is_directory(src))
                fs::copy(src, target,
                    fs::copy_options::recursive |
                    fs::copy_options::skip_existing, error);
            else
                fs::copy_file(src, target,
                    fs::copy_options::skip_existing, error);

            if (error)
                std::cerr << "[ContentBrowser] Import failed for "
                << src << ": " << error.message() << "\n";
            else
                anyChange = true;
        }

        if (anyChange)
        {
            m_externalDropDest.clear();
            Refresh();
        } 
    }

    bool ContentBrowser::ContainsGridPoint(float x, float y) const
    {
        return x >= m_gridX && x <= m_gridX + m_gridWidth &&
               y >= m_gridY && y <= m_gridY + m_gridHeight;
    }

    void ContentBrowser::Draw()
    {
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
        m_gridX = gridPos[0];  m_gridY = gridPos[1];
        m_gridWidth = gridSize[0]; m_gridHeight = gridSize[1];
        DrawAssetGrid();
        m_gui.EndChildPanel();

        if (!m_hoveredDestThisFrame.empty())
            m_externalDropDest = m_hoveredDestThisFrame;

        if (!m_selectedPath.empty() && m_renamingPath.empty())
        {
            if (m_gui.IsKeyPressed(UIKey::Delete))
            {
                DeleteSelected();
            }
        }

        if (m_gui.IsWindowHovered() && !m_gui.IsAnyItemHovered() &&
            (m_gui.IsMouseReleased(MouseButton::Left) || m_gui.IsMouseReleased(MouseButton::Right)))
        {
            m_selectedPath.clear();
            m_renamingPath.clear();
        }

        m_gui.EndPanel();
    }

    fs::path ContentBrowser::GetExternalDropDest(float mouseX, float mouseY) const
    {
        for (const auto& rect : m_breadcrumbRects)
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
        fs::path displayPath = (m_searchBuffer[0] != '\0' && !m_selectedPath.empty())
            ? m_selectedPath.parent_path() : m_currentPath;

        fs::path relative = fs::relative(displayPath, m_rootPath.parent_path());
        fs::path accumulated = m_rootPath.parent_path();

        m_breadcrumbRects.clear();

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

    void ContentBrowser::DrawFolderTree(const fs::path& dir)
    {
        if (!fs::exists(dir)) return;

        m_gui.PushStyleVariable(StyleVariable::FramePadding, 4.f, 5.f);
        m_gui.PushStyleVariable(StyleVariable::ItemSpacing, 6.f, 6.f);
        m_gui.PushStyleVariable(StyleVariable::IndentSpacing, 18.f);
        m_gui.PushFont(FontID::Default);

        for (const auto& entry : fs::directory_iterator(dir))
            if (entry.is_directory()) DrawFolderTreeEntry(entry);

        m_gui.PopFont();
        m_gui.PopStyleVariable(); // IndentSpacing
        m_gui.PopStyleVariable(); // ItemSpacing
        m_gui.PopStyleVariable(); // FramePadding
    }

    void ContentBrowser::DrawFolderTreeEntry(const fs::directory_entry& entry)
    {
        const fs::path& path = entry.path();
        bool isRenaming = (m_renamingPath == path);

        bool open = DrawTreeNode(path, isRenaming);
        HandleTreeNodeInput(path, isRenaming);

        AssetEntry assetEntry = ClassifyEntry(entry);
        DrawEntryContextMenu(assetEntry);

        if (open) { DrawFolderTree(path); m_gui.EndTreeNode(); }
    }

    bool ContentBrowser::DrawTreeNode(const fs::path& path, bool isRenaming)
    {
        auto flags = BuildTreeNodeFlags(path);
        std::string label = isRenaming ? "##renaming_" + path.string() : path.filename().string();

        m_gui.PushColor(Apex::UserInterface::StyleColor::Text, { 1.f, 0.85f, 0.4f, 1.f });
        bool open = m_gui.BeginTreeNode(label, flags);
        m_gui.PopColor();
        return open;
    }

    void ContentBrowser::HandleTreeNodeInput(const fs::path& path, bool isRenaming)
    {
        if (isRenaming) { DrawTreeNodeRename(); return; }

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
                MoveEntry(fs::path(static_cast<const char*>(data)), path);
            m_hoveredDestThisFrame = path;
            m_gui.EndDropTarget();
        }
    }

    Apex::UserInterface::TreeNodeFlags ContentBrowser::BuildTreeNodeFlags(const fs::path& path) const
    {
        using Apex::UserInterface::TreeNodeFlags;
        bool hasSubDirs = false;
        for (const auto& sub : fs::directory_iterator(path))
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

        const std::vector<AssetEntry>& source = searching ? m_searchEntries : m_entries;

        LibMath::Vector2 availableSize = m_gui.GetAvailableSize();
        int columns = std::max(1, static_cast<int>(availableSize[0] / (m_tileSize + 16.f)));
        int col = 0;

        m_pendingNavigate.clear();

        for (const auto& entry : source)
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
                ImportFiles(std::move(paths));
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

        m_gui.EndPopup();
    }

    void ContentBrowser::DrawTile(const AssetEntry& entry)
    {
        m_gui.PushID(entry.m_path.string());

        bool isSelected = (!m_selectedPath.empty() && m_selectedPath == entry.m_path);
        if (isSelected)
            m_gui.PushColor(Apex::UserInterface::StyleColor::ChildBackground,
                { 0.26f, 0.42f, 0.60f, 0.5f });

        m_gui.BeginChildPanel("##tile", m_tileSize, m_tileSize + 26.f, false);

        LibMath::Vector2 pos = m_gui.GetCursorPos();
        float iconSize = m_tileSize - 6.f * 2.f;

        DrawTileIcon(entry, pos);
        DrawTileInteraction(entry, pos, iconSize);
        // Show rename input instead of the static label while renaming this entry
        if (m_renamingPath == entry.m_path)
            DrawRenameInline(entry, pos);
        else
            DrawTileLabel(entry, pos);

        m_gui.EndChildPanel();

        if (isSelected)
            m_gui.PopColor();

        m_gui.PopID();
    }

    void ContentBrowser::DrawTileIcon(const AssetEntry& entry, LibMath::Vector2 pos)
    {
        using Apex::UserInterface::PackColor;

        Apex::UserInterface::IDrawList* drawList = m_gui.GetDrawList();
        float pad = 6.f;
        float iconSize = m_tileSize - pad * 2.f;
        uint32_t col = TypeColor(entry.m_type);

        // Dark rounded background
        drawList->DrawRectFilled(
            LibMath::Vector2(pos[0] + pad, pos[1] + pad),
            LibMath::Vector2(pos[0] + pad + iconSize, pos[1] + pad + iconSize),
            PackColor(30, 30, 35), 8.f);

        // Colored accent bar at top
        drawList->DrawRectFilled(
            LibMath::Vector2(pos[0] + pad, pos[1] + pad),
            LibMath::Vector2(pos[0] + pad + iconSize, pos[1] + pad + 6.f),
            col, 8.f);

        // Type label centered in icon box
        const char* label = TypeLabel(entry.m_type);
        LibMath::Vector2 labelSize = drawList->CalculateTextSize(label);
        drawList->DrawText(
            LibMath::Vector2(
                pos[0] + pad + (iconSize - labelSize[0]) * 0.5f,
                pos[1] + pad + (iconSize - labelSize[1]) * 0.5f + 4.f),
            col, label);
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
                m_onLoad(entry);
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
                MoveEntry(fs::path(static_cast<const char*>(data)), entry.m_path);
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
            auto bytes = fs::file_size(entry.m_path, error);
            if (!error)
                m_gui.TextDisabled(std::to_string(bytes / 1024) + " KB");
        }

        m_gui.EndTooltip();
    }

    void ContentBrowser::DrawTileLabel(const AssetEntry& entry, LibMath::Vector2 pos)
    {
        // Show stem only
        std::string name = entry.m_isFolder ? entry.m_path.filename().string() : entry.m_path.stem().string();

        if (name.size() > 18)
            name = name.substr(0, 17) + "...";

        LibMath::Vector2 textSize = m_gui.GetDrawList()->CalculateTextSize(name.c_str());
        float centeredX = pos[0] + (m_tileSize - textSize[0]) * 0.5f;
        float labelY = pos[1] + m_tileSize + 4.f;

        // Invisible button over the label area so we can detect double-click
        m_gui.SetCursorPos(LibMath::Vector2(centeredX, labelY));
        m_gui.InvisibleButton("##label", textSize);
        if (m_gui.IsItemDoubleClicked())
            BeginRename(entry.m_path);

        // Draw the text on top of the button
        m_gui.GetDrawList()->DrawText(LibMath::Vector2(centeredX, labelY), 0xFFAAAAAA, name);
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
        {
            if (m_gui.MenuItem("Open")) NavigateTo(entry.m_path);
        }
        else if (m_gui.MenuItem("Load") && m_onLoad)
        {
            m_onLoad(entry);
        }
               
        m_gui.Separator();

        if (m_gui.MenuItem("Rename"))                          BeginRename(entry.m_path);
        if (!entry.m_isFolder && m_gui.MenuItem("Duplicate"))  DuplicateEntry(entry);
        if (m_gui.MenuItem("Copy Path"))                       m_gui.SetClipboardText(entry.m_path.string());

        m_gui.Separator();

        if (m_gui.MenuItem("Reveal in Explorer"))  RevealInExplorer(entry.m_path);

        m_gui.Separator();

        if (m_gui.MenuItem("Delete")) { m_selectedPath = entry.m_path; DeleteSelected(); }

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
                ImportFiles(std::move(paths));
        }

        if (m_gui.MenuItem("Import Folder..."))
        {
            auto folder = m_window.OpenFolderDialog();
            if (!folder.empty())
                ImportFiles({ folder });
        }

        m_gui.Separator();

        if (m_gui.MenuItem("Refresh"))
            Refresh();

        if (m_gui.MenuItem("Reveal in Explorer"))
        {
            std::system(("explorer \"" + m_currentPath.string() + "\"").c_str());
        }

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

    void ContentBrowser::DrawBreadcrumbPart(const fs::path& accumulated, const std::string& label, int idx)
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

        if (fs::is_directory(accumulated))
            m_breadcrumbRects.push_back({ accumulated, buttonPos[0], buttonPos[1], buttonWidth, buttonHeight });

        m_gui.PushID(idx);
        bool clicked = m_gui.InvisibleButton("##bc", LibMath::Vector2(buttonWidth, buttonHeight));
        bool hovered = m_gui.IsItemHovered();
        m_gui.PopID();

        drawList->DrawText(LibMath::Vector2(buttonPos[0] + PAD_X, buttonPos[1]),
            hovered ? COL_HOVERED : COL_NORMAL, label);

        if (!fs::is_directory(accumulated)) return;

        if (clicked) NavigateTo(accumulated);
        if (hovered) m_hoveredDestThisFrame = accumulated;

        if (m_gui.BeginDropTarget())
        {
            const void* data = m_gui.AcceptDragPayload("ASSET_PATH");
            if (data)
                MoveEntry(fs::path(static_cast<const char*>(data)), accumulated);
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