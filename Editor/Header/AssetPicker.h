#ifndef ASSET_PICKER
#define ASSET_PICKER

#include "IResource.h"
#include "ResourceManager.h"
#include "UI.h"
#include "Mesh.h"
#include "Texture.h"
#include "Material.h"
#include "Command/SetAssetCommand.h"
#include "CommandManager.h"

#include <vector>
#include <filesystem>

namespace Apex::Resources
{
    template<typename T>
    class AssetPicker
    {
    public:
        AssetPicker(Apex::UserInterface::IGUI& gui,
            Apex::Resources::ResourceManager& resourceManager) 
            : m_gui(gui), m_resourceManager(resourceManager) {}

        void Open(ResourceHandle<T>* target);

        void Draw();

        bool HasNewAsset() { return m_newAsset; }
        void ResetNewAsset() { m_newAsset = false; }

    private:
        void ScanAssets();
        static bool MatchesType(const std::filesystem::path& path);

        bool m_isOpen = false;
        bool m_newAsset = false;
        ResourceHandle<T>* m_target = nullptr;
        std::vector<std::string> m_cachedPaths;

        Apex::UserInterface::IGUI& m_gui;
        Apex::Resources::ResourceManager& m_resourceManager;
    };

    template<typename T>
    inline void AssetPicker<T>::Open(ResourceHandle<T>* target)
    {
        m_isOpen = true;
        m_target = target;
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
    }

    template<typename T>
    inline bool AssetPicker<T>::MatchesType(const std::filesystem::path& path)
    {
        std::string ext = path.extension().string();
        // Lowercase for case-insensitive comparison
        for (auto& c : ext) c = static_cast<char>(::tolower(c));

        if constexpr (std::is_same_v<T, Mesh>)
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
    inline void AssetPicker<T>::Draw()
    {
        if (!m_isOpen || !m_target) return;

        m_gui.BeginModalBlock("Select Asset");
        
        m_gui.BeginPanel("Select Asset", &m_isOpen, false, true);

        if (m_gui.Button("Refresh"))
            ScanAssets();

        m_gui.Separator();

        for (const auto& pathStr : m_cachedPaths)
        {
            if (m_gui.Button(pathStr.c_str()))
            {
                if (!m_target->IsValid() || (*m_target)->GetPath() != pathStr)
                {
                    ResourceHandle<T> newVal = m_resourceManager.CreateAsync<T>(pathStr);

                    CtrlZ::CommandManager::Get().Execute(std::make_unique<CtrlZ::SetAssetCommand<T>>(m_target, *m_target, newVal));
                    m_newAsset = true;
                }
                m_isOpen = false;
            }
        }

        m_gui.EndPanel();

        m_gui.EndModalBlock();
    }
}

#endif // !ASSET_PICKER
