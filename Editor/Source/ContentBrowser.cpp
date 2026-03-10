#include "ContentBrowser.h"

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
        case AssetType::Sound:   return PackColor(255, 200, 80);   // amber
        case AssetType::Level:   return PackColor(220, 120, 255);  // purple
        case AssetType::Folder:  return PackColor(255, 210, 100);  // gold
        default:                 return PackColor(160, 160, 160);  // grey
        }
    }

    ContentBrowser::ContentBrowser(Apex::UserInterface::IGUI& gui, const fs::path& rootPath)
        : m_gui(gui), m_rootPath(fs::weakly_canonical(rootPath)), m_currentPath(fs::weakly_canonical(rootPath))
    {
        Refresh();
    }

    void ContentBrowser::Refresh() { ScanCurrentDirectory(); }

    void ContentBrowser::NavigateTo(const fs::path& dir)
    {
        m_currentPath = fs::weakly_canonical(dir);
        m_selected = nullptr;
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
            AssetEntry assetEntry;
            assetEntry.m_path = entry.path();
            assetEntry.m_isFolder = entry.is_directory();

            if (assetEntry.m_isFolder)
            {
                assetEntry.m_type = AssetType::Folder;
                folders.push_back(assetEntry);
            }
            else
            {
                std::string ext = entry.path().extension().string();
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                assetEntry.m_type = ClassifyExtension(ext);
                files.push_back(assetEntry);
            }
        }
		// Sort folders and files separately, then concatenate (folders first).
        auto byName = [](const AssetEntry& a, const AssetEntry& b)
            {
                return a.m_path.filename() < b.m_path.filename();
            };
        std::sort(folders.begin(), folders.end(), byName);
        std::sort(files.begin(), files.end(), byName);

        m_entries.insert(m_entries.end(), folders.begin(), folders.end());
        m_entries.insert(m_entries.end(), files.begin(), files.end());
    }

    void ContentBrowser::Draw()
    {
        m_gui.BeginPanel("Content Browser");

        DrawToolbar();
        m_gui.Separator();
        DrawBreadcrumb();
        m_gui.Separator();

        if (m_showTree)
        {
            m_gui.BeginChildPanel("##cb_tree", 200.f, 0.f, true);
            DrawFolderTree(m_rootPath);
            m_gui.EndChildPanel();
            m_gui.SameLine();
        }

        m_gui.BeginChildPanel("##cb_grid", 0.f, 0.f, false);
        DrawAssetGrid();
        m_gui.EndChildPanel();

        m_gui.EndPanel();
    }

    void ContentBrowser::DrawToolbar()
    {
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
        m_gui.SetNextItemWidth(120.f);
        m_gui.SliderFloat("Tile size", &m_tileSize, 48.f, 160.f);
    }

    void ContentBrowser::DrawBreadcrumb()
    {
		// Breadcrumb navigation: split current path relative to root and render each segment as a button.
        fs::path relative = fs::relative(m_currentPath, m_rootPath.parent_path());
        fs::path accumulated = m_rootPath.parent_path();

        bool first = true;
        for (const auto& part : relative)
        {
            accumulated /= part;

            if (!first)
            {
                m_gui.SameLine();
                m_gui.TextDisabled("/");
                m_gui.SameLine();
            }
            first = false;

            // Transparent button style for clickable path segment
            m_gui.PushColor(Apex::UserInterface::StyleColor::Button, { 0.f, 0.f, 0.f, 0.f });
            if (m_gui.SmallButton(part.string()))
                if (fs::is_directory(accumulated))
                    NavigateTo(accumulated);
            m_gui.PopColor();
        }
    }

    void ContentBrowser::DrawFolderTree(const fs::path& dir)
    {
        if (!fs::exists(dir)) return;

        for (const auto& entry : fs::directory_iterator(dir))
        {
            if (!entry.is_directory()) continue;

            // Check for sub-folders to decide leaf flag
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

            // Gold color for folder names
            m_gui.PushColor(Apex::UserInterface::StyleColor::Text, { 1.f, 0.85f, 0.4f, 1.f });
            bool open = m_gui.BeginTreeNode(entry.path().filename().string(), flags);
            m_gui.PopColor();

            if (m_gui.IsItemClicked())
                NavigateTo(entry.path());

            if (open)
            {
                DrawFolderTree(entry.path());
                m_gui.EndTreeNode();
            }
        }
    }

    void ContentBrowser::DrawAssetGrid()
    {
        LibMath::Vector2 availableSize = m_gui.GetAvailableSize();
        int columns = std::max(1, static_cast<int>(availableSize[0] / (m_tileSize + 16.f)));
        int col = 0;

        for (auto& entry : m_entries)
        {
            // Search filter
            if (m_searchBuffer[0] != '\0')
            {
                std::string name = entry.m_path.filename().string();
                std::string filter = m_searchBuffer;
                std::transform(name.begin(), name.end(), name.begin(), ::tolower);
                std::transform(filter.begin(), filter.end(), filter.begin(), ::tolower);
                if (name.find(filter) == std::string::npos)
                    continue;
            }

            if (col > 0 && col % columns != 0)
                m_gui.SameLine();

            DrawTile(entry);
            col++;
        }
    }

    void ContentBrowser::DrawTile(AssetEntry& entry)
    {
        m_gui.PushID(entry.m_path.string());

        bool isSelected = (m_selected && m_selected->m_path == entry.m_path);
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
        // Places an invisible button over the icon, handles click / double-click,
        // shows the context menu, and triggers the tooltip.
        float pad = 6.f;

        m_gui.SetCursorPos(LibMath::Vector2(pos[0] + pad, pos[1] + pad));
        m_gui.InvisibleButton("##btn", LibMath::Vector2(iconSize, iconSize));

        if (m_gui.IsItemClicked())
            m_selected = &entry;

        if (m_gui.IsItemDoubleClicked())
        {
            if (entry.m_isFolder)
                NavigateTo(entry.m_path);
            else if (m_onLoad)
                m_onLoad(entry);
        }

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
        m_gui.SetCursorPos(LibMath::Vector2(pos[0], pos[1] + m_tileSize + 2.f));

        std::string name = entry.m_path.filename().string();
        if (name.size() > 12)
            name = name.substr(0, 11) + "~";

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