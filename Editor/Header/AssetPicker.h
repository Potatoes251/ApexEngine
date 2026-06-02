#ifndef ASSET_PICKER
#define ASSET_PICKER

#include "IResource.h"
#include "ResourceManager.h"
#include "UI.h"
#include "IDrawList.h"
#include "EditorIcon.h"
#include "ThumbnailRenderer.h"
#include "Model.h"
#include "Texture.h"
#include "Material.h"
#include "Command/SetAssetCommand.h"
#include "CommandManager.h"

#include <vector>
#include <filesystem>

using namespace Apex::Editor;
using namespace Apex::UserInterface;

namespace Apex::Resources
{
    inline constexpr float PICKER_THUMB = 72.f;
    inline constexpr float PICKER_LABEL_H = 20.f;
    inline constexpr float PICKER_CELL_H = PICKER_THUMB + PICKER_LABEL_H;
    inline constexpr float PICKER_SPACING = 10.f;
    inline constexpr float PICKER_SCROLL_W = 16.f;

    inline int PickerColumnCount(float availableWidth)
    {
        float usable = availableWidth - PICKER_SCROLL_W;
        return std::max(1, static_cast<int>(
            (usable + PICKER_SPACING) / (PICKER_THUMB + PICKER_SPACING)));
    }

    inline uint32_t PickerThumb(const std::string& path, ThumbnailRenderer* thumbnails)
    {
        std::string ext = Fs::path(path).extension().string();
        for (auto& c : ext) c = static_cast<char>(::tolower(c));

        if (ext == ".mesh")
            return thumbnails ? thumbnails->GetOrRender(path) : 0;

        if (ext == ".mat")
            return thumbnails ? thumbnails->GetOrRenderMaterial(path) : 0;

        return EditorIcon::Get(path);
    }

    inline bool DrawPickerCell(IGUI* gui, const std::string& pathStr,
        uint32_t thumb, float thumbSize = PICKER_THUMB,
        float labelH = PICKER_LABEL_H)
    {
        float cellH = thumbSize + labelH;
        bool  picked = gui->InvisibleButton("##cell", { thumbSize, cellH });

        LibMath::Vector2 bMin = gui->GetItemRectMin();
        LibMath::Vector2 bMax = gui->GetItemRectMax();
        IDrawList* drawList = gui->GetDrawList();

        bool     hovered = gui->IsItemHovered();
		uint32_t bgCol = hovered ? 0xFF2A3A4A : 0xFF1A1E26; // dark grey-blue or darker variant on hover
        drawList->DrawRectFilled(bMin, bMax, bgCol, 4.f);
        drawList->DrawRect(bMin, bMax, 0xFF333D4D, 4.f, 1.f); // dark grey

        LibMath::Vector2 thumbMax = { bMin[0] + thumbSize, bMin[1] + thumbSize };
        if (thumb != 0)
        {
            gui->DrawImageBackground(thumb, bMin, thumbMax);
        }
        else
        {
			drawList->DrawRectFilled(bMin, thumbMax, 0xFF141820, 0.f); // darker grey for missing thumbnail
            std::string ext = Fs::path(pathStr).extension().string();
            LibMath::Vector2 textSize = drawList->CalculateTextSize(ext);
            drawList->DrawText(
                { bMin[0] + std::floor((thumbSize - textSize[0]) * 0.5f),
                 bMin[1] + std::floor((thumbSize - textSize[1]) * 0.5f) }, 0xFF88CCFF, ext); // light blue
        }

        float stripY = bMin[1] + thumbSize;
		drawList->DrawRectFilled({ bMin[0], stripY }, bMax, 0xEE0D1117, 0.f); // semi-transparent very dark grey for label background

        std::string stem = Fs::path(pathStr).stem().string();
        if (stem.size() > 7) stem = stem.substr(0, 6) + "~";
        LibMath::Vector2 textSize = drawList->CalculateTextSize(stem);
        drawList->DrawText(
            { bMin[0] + std::floor((thumbSize - textSize[0]) * 0.5f),
             stripY + std::floor((labelH - textSize[1]) * 0.5f) }, 0xFFE8E8E8, stem); // light grey

        if (hovered)
        {
            gui->BeginTooltip();
            gui->Text(pathStr);
            gui->EndTooltip();
        }

        return picked;
    }

    template<typename T>
    class AssetPicker
    {
    public:
        AssetPicker(Apex::UserInterface::IGUI* gui,
            Apex::Resources::ResourceManager* resourceManager)
            : m_gui(gui), m_resourceManager(resourceManager) {
        }

        void Open(ResourceHandle<T>* target);

        void SetThumbnailRenderer(ThumbnailRenderer* t) { m_thumbnails = t; }

        void Draw();

        bool HasNewAsset() { return m_newAsset; }
        void ResetNewAsset() { m_newAsset = false; }

    private:
        void ScanAssets();
        static bool MatchesType(const std::filesystem::path& path);

        void DrawToolbar();

        std::string DrawGrid(int cols, const std::string& filterStr);

        void CommitSelection(const std::string& pickedPath);

        bool m_isOpen = false;
        bool m_newAsset = false;
        ResourceHandle<T>* m_target = nullptr;
        ThumbnailRenderer* m_thumbnails = nullptr;
        std::vector<std::string> m_cachedPaths;
        char m_filter[256] = {};

        Apex::UserInterface::IGUI* m_gui;
        Apex::Resources::ResourceManager* m_resourceManager;

        static constexpr float THUMB_SIZE = 72.f;
        static constexpr float LABEL_HEIGHT = 20.f;
        static constexpr float CELL_PAD = 6.f;
    };

    template<typename T>
    inline void AssetPicker<T>::Open(ResourceHandle<T>* target)
    {
        m_isOpen = true;
        m_target = target;
        m_filter[0] = '\0';
        ScanAssets();   // scan once on open, not every frame
    }

    template<typename T>
    inline void AssetPicker<T>::ScanAssets()
    {
        m_cachedPaths.clear();
        for (auto& entry : std::filesystem::recursive_directory_iterator("Assets/"))
        {
            if (!entry.is_regular_file()) continue;
            if (MatchesType(entry.path()))
                m_cachedPaths.push_back(entry.path().generic_string());
        }

        for (auto& entry : std::filesystem::recursive_directory_iterator("ApexAssets/"))
        {
            if (!entry.is_regular_file()) continue;
            if (MatchesType(entry.path()))
                m_cachedPaths.push_back(entry.path().generic_string());
        }
    }

    template<typename T>
    inline bool AssetPicker<T>::MatchesType(const std::filesystem::path& path)
    {
        std::string ext = path.extension().string();
        // Lowercase for case-insensitive comparison
        for (auto& c : ext) c = static_cast<char>(::tolower(c));

        if constexpr (std::is_same_v<T, Model>)
            return ext == ".mesh";
        if constexpr (std::is_same_v<T, Rendering::Material>)
            return ext == ".mat";
        else if constexpr (std::is_same_v<T, Texture>)
            return ext == ".png" || ext == ".jpg" || ext == ".jpeg"
            || ext == ".hdr" || ext == ".tga";
        else
            return true;  // unknown type - show everything
    }

    template<typename T>
    inline void AssetPicker<T>::DrawToolbar()
    {
        m_gui->SetNextItemWidth(-80.f);
        m_gui->InputText("##filter", m_filter, sizeof(m_filter));
        m_gui->SameLine(0.f, 4.f);
        if (m_gui->Button("Refresh")) ScanAssets();
        m_gui->Separator();
    }

    template<typename T>
    inline std::string AssetPicker<T>::DrawGrid(int cols, const std::string& filterStr)
    {
        m_gui->BeginChildPanel("##ap_grid", 0.f, -30.f, false);

        int col = 0;
        for (const auto& pathStr : m_cachedPaths)
        {
            if (!filterStr.empty())
            {
                std::string lower = pathStr;
                for (auto& c : lower) c = static_cast<char>(::tolower(c));
                if (lower.find(filterStr) == std::string::npos) continue;
            }

            if (col > 0 && col % cols != 0)
                m_gui->SameLine(0.f, PICKER_SPACING);

            m_gui->PushID(pathStr.c_str());
            uint32_t thumb = PickerThumb(pathStr, m_thumbnails);
            bool picked = DrawPickerCell(m_gui, pathStr, thumb);
            m_gui->PopID();

            if (picked)
            {
                m_gui->EndChildPanel();
                return pathStr;
            }
            col++;
        }

        if (m_cachedPaths.empty())
            m_gui->TextDisabled("No assets found.");

        m_gui->EndChildPanel();
        return {};
    }

    template<typename T>
    inline void AssetPicker<T>::CommitSelection(const std::string& pickedPath)
    {
        ResourceHandle<T> newVal = m_resourceManager->CreateAsync<T>(pickedPath);
        CtrlZ::CommandManager::Get().Execute(std::make_unique<CtrlZ::SetAssetCommand<T>>(m_target, *m_target, newVal));
        m_newAsset = true;
        m_isOpen = false;
    }

    template<typename T>
    inline void AssetPicker<T>::Draw()
    {
        if (!m_isOpen) return;

        m_gui->BeginModalBlock("Select Asset");
        m_gui->BeginPanel("Select Asset", &m_isOpen, false, true);

        DrawToolbar();

        float availableWidth = m_gui->GetAvailableSize()[0];
        int   cols = PickerColumnCount(availableWidth);

        std::string filterStr(m_filter);

        std::string picked = DrawGrid(cols, filterStr);
        if (!picked.empty())
        {
            CommitSelection(picked);
            m_gui->EndPanel();
            m_gui->EndModalBlock();
            return;
        }

        m_gui->EndPanel();
        m_gui->EndModalBlock();
    }
}

#endif // !ASSET_PICKER
