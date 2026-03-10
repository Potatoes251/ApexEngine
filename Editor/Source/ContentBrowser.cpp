#include "ContentBrowser.h"

#include <iostream>

namespace Apex::Editor
{
    AssetType ContentBrowser::ClassifyExtension(const std::string& ext)
    {
        if (ext == ".obj" || ext == ".fbx")                  return AssetType::Mesh;
        if (ext == ".jpg" || ext == ".png" || ext == ".tga") return AssetType::Texture;
        if (ext == ".ogg" || ext == ".wav" || ext == ".mp3") return AssetType::Sound;
        if (ext == ".level")                                 return AssetType::Level;
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

    ContentBrowser::ContentBrowser(Apex::UserInterface::IGUI& gui, const fs::path& rootPath)
        : m_gui(gui), m_rootPath(fs::weakly_canonical(rootPath)), m_currentPath(fs::weakly_canonical(rootPath))
    {
        Refresh();
    }

    void ContentBrowser::Refresh() 
    { 
        m_selectedPath.clear();
        ScanCurrentDirectory(); 
    }

    void ContentBrowser::NavigateTo(const fs::path& dir)
    {
        m_currentPath = fs::weakly_canonical(dir);
        m_selectedPath.clear();
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
        m_gui.BeginPanel("Content Browser");

        DrawToolbar();
        m_gui.Separator();
        DrawBreadcrumb();
        m_gui.Separator();

        m_hoveredDestThisFrame.clear();

        if (m_showTree)
        {
            m_gui.BeginChildPanel("##cb_tree", 200.f, 0.f, true);
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
        // Capture the Y of this toolbar line before drawing anything
        float toolbarY = m_gui.GetCursorPos()[1];

        // Search box
        m_gui.SetNextItemWidth(200.f);
        m_gui.InputText("##cb_search", m_searchBuffer, sizeof(m_searchBuffer), "Search assets...");

        if (m_searchBuffer[0] != '\0')
        {
            m_gui.SameLine();
            if (m_gui.SmallButton("x"))
                std::memset(m_searchBuffer, 0, sizeof(m_searchBuffer));
        }

        m_gui.SameLine();
        if (m_gui.SmallButton("Refresh"))
            Refresh();

        m_gui.SameLine();
        if (m_gui.SmallButton(m_showTree ? "Hide Folder Tab" : "Show Folder Tab"))
            m_showTree = !m_showTree;

        if (m_currentPath != m_rootPath)
        {
            m_gui.SameLine();
            if (m_gui.SmallButton("<- Back"))
                NavigateTo(m_currentPath.parent_path());
        }

        m_gui.SameLine();
		m_gui.TextDisabled("Tile Size:");
        m_gui.SameLine();
        m_gui.SetNextItemWidth(120.f);
        m_gui.SliderInt("##tilesize", &m_tileSize, 80, 160);
        
    }

    void ContentBrowser::DrawBreadcrumb()
    {
        fs::path displayPath = (m_searchBuffer[0] != '\0' && !m_selectedPath.empty()) ? 
            m_selectedPath.parent_path() :
            m_currentPath;

        fs::path relative = fs::relative(displayPath, m_rootPath.parent_path());
        fs::path accumulated = m_rootPath.parent_path();

        Apex::UserInterface::IDrawList* drawList = m_gui.GetDrawList();
        constexpr uint32_t colNormal = 0xFFCCCCCC;
        constexpr uint32_t colHovered = 0xFFFFFFFF;
        constexpr float    padX = 4.f;

        bool first = true;
        int  idx = 0;

        m_breadcrumbRects.clear();

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

            LibMath::Vector2 textSize = drawList->CalculateTextSize(part.string());
            float buttonWidth = textSize[0] + padX * 2.f;
            float buttonHeight = textSize[1];

            LibMath::Vector2 buttonPos = m_gui.GetCursorPos();

            if (fs::is_directory(accumulated))
                m_breadcrumbRects.push_back({ accumulated, buttonPos[0], buttonPos[1], buttonWidth, buttonHeight });

            m_gui.PushID(idx++);
            bool clicked = m_gui.InvisibleButton("##bc", LibMath::Vector2(buttonWidth, buttonHeight));
            bool hovered = m_gui.IsItemHovered();

            uint32_t col = hovered ? colHovered : colNormal;
            drawList->DrawText(LibMath::Vector2(buttonPos[0] + padX, buttonPos[1]), col, part.string());

            if (clicked && fs::is_directory(accumulated))
                NavigateTo(accumulated);

            if (fs::is_directory(accumulated))
            {
                if (hovered)
                    m_hoveredDestThisFrame = accumulated;

                if (m_gui.BeginDropTarget())
                {
                    const void* data = m_gui.AcceptDragPayload("ASSET_PATH");
                    if (data)
                        MoveEntry(fs::path(static_cast<const char*>(data)), accumulated);
                    m_hoveredDestThisFrame = accumulated;
                    m_gui.EndDropTarget();
                }
            }
            m_gui.PopID();
        }
    }

    void ContentBrowser::DrawFolderTree(const fs::path& dir)
    {
        if (!fs::exists(dir)) return;

        for (const auto& entry : fs::directory_iterator(dir))
        {
            if (!entry.is_directory()) continue;

            bool hasSubDirs = false;
            for (const auto& sub : fs::directory_iterator(entry.path()))
            {
                if (sub.is_directory())
                {
                    hasSubDirs = true;
                    break;
                }
            }
                
            using Apex::UserInterface::TreeNodeFlags;
            TreeNodeFlags flags = TreeNodeFlags::OpenOnArrow | TreeNodeFlags::SpanAvailWidth;

            if (!hasSubDirs)                       flags = flags | TreeNodeFlags::Leaf;
            if (entry.path() == m_currentPath)     flags = flags | TreeNodeFlags::Selected;

            m_gui.PushColor(Apex::UserInterface::StyleColor::Text, { 1.f, 0.85f, 0.4f, 1.f });
            bool open = m_gui.BeginTreeNode(entry.path().filename().string(), flags);
            m_gui.PopColor();

            if (m_gui.IsItemClicked())
                NavigateTo(entry.path());

            if (m_gui.IsItemHovered())
                m_hoveredDestThisFrame = entry.path();

            if (m_gui.BeginDropTarget())
            {
                const void* data = m_gui.AcceptDragPayload("ASSET_PATH");
                if (data)
                    MoveEntry(fs::path(static_cast<const char*>(data)), entry.path());
                m_hoveredDestThisFrame = entry.path();
                m_gui.EndDropTarget();
            }

            if (open)
            {
                DrawFolderTree(entry.path());
                m_gui.EndTreeNode();
            }
        }
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

        for (auto& entry : const_cast<std::vector<AssetEntry>&>(source))
        {
            if (col > 0 && col % columns != 0)
                m_gui.SameLine();
            DrawTile(entry);
            col++;
        }

        if (!m_pendingNavigate.empty())
        {
            NavigateTo(m_pendingNavigate);
        }
    }

    void ContentBrowser::DrawTile(AssetEntry& entry)
    {
        m_gui.PushID(entry.m_path.string());

        bool isSelected = (!m_selectedPath.empty() && m_selectedPath == entry.m_path);
        if (isSelected)
            m_gui.PushColor(Apex::UserInterface::StyleColor::ChildBackground,
                { 0.26f, 0.42f, 0.60f, 0.5f });

        m_gui.BeginChildPanel("##tile", m_tileSize, m_tileSize + 22.f, false);

        LibMath::Vector2 pos = m_gui.GetCursorPos();
        float iconSize = m_tileSize - 6.f * 2.f;

        DrawTileIcon(entry, pos);
        DrawTileInteraction(entry, pos, iconSize);
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


    void ContentBrowser::DrawTileInteraction(AssetEntry& entry, LibMath::Vector2 pos, float iconSize)
    {
        float pad = 6.f;

        m_gui.SetCursorPos(LibMath::Vector2(pos[0] + pad, pos[1] + pad));
        m_gui.InvisibleButton("##btn", LibMath::Vector2(iconSize, iconSize));

        if (m_gui.IsItemClicked())
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

        DrawContextMenu(entry);

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
        std::string name = entry.m_path.filename().string();
        if (name.size() > 18)
            name = name.substr(0, 17) + "...";

        LibMath::Vector2 textSize = m_gui.GetDrawList()->CalculateTextSize(name.c_str());
        float centeredX = pos[0] + (m_tileSize - textSize[0]) * 0.5f;
        m_gui.SetCursorPos(LibMath::Vector2(centeredX, pos[1] + m_tileSize + 2.f));
        m_gui.TextDisabled(name);
    }

    void ContentBrowser::DrawContextMenu(const AssetEntry& entry)
    {
        if (!m_gui.BeginContextMenu()) return;

        m_gui.TextDisabled(entry.m_path.filename().string());
        m_gui.Separator();

        if (!entry.m_isFolder && m_gui.MenuItem("Load"))
            if (m_onLoad) m_onLoad(entry);

        if (m_gui.MenuItem("Reveal in Explorer"))
        {
			// Note: this is Windows-specific. 
            std::system(("explorer /select,\"" + entry.m_path.string() + "\"").c_str());
        }

        m_gui.Separator();
        if (m_gui.MenuItem("Delete"))
        {
            std::error_code ec;
            if (entry.m_isFolder) fs::remove_all(entry.m_path, ec);
            else                  fs::remove(entry.m_path, ec);
            Refresh();
        }

        m_gui.EndContextMenu();
    }

}