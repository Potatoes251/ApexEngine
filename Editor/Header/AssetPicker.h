#ifndef ASSET_PICKER
#define ASSET_PICKER

#include "IResource.h"
#include "ResourceManager.h"
#include "UI.h"

#include <vector>
#include <filesystem>

namespace Apex::Resources
{
    enum AssetType
    {
        MeshAsset,
        TextureAsset
    };

    struct AssetInfo
    {
        std::string path;
        AssetType type;
    };

    template<typename T>
    class AssetPicker
    {
    public:
        AssetPicker(Apex::UserInterface::IGUI& gui,
            Apex::Resources::ResourceManager& resourceManager) 
            : m_gui(gui), m_resourceManager(resourceManager) {}

        void Open(ResourceHandle<T>* target);

        void Draw();

    private:
        bool m_isOpen = false;
        ResourceHandle<T>* m_target = nullptr;
        Apex::UserInterface::IGUI& m_gui;
        Apex::Resources::ResourceManager& m_resourceManager;
        std::vector<AssetInfo> m_assets;
    };

    template<typename T>
    inline void AssetPicker<T>::Open(ResourceHandle<T>* target)
    {
        m_isOpen = true;
        m_target = target;
    }

    template<typename T>
    inline void AssetPicker<T>::Draw()
    {
        if (!m_isOpen)
            return;

        m_gui.BeginPanel("Select Asset", &m_isOpen);

        for (auto& entry : std::filesystem::recursive_directory_iterator("Assets/"))
        {
            if (!entry.is_regular_file())
                continue;

            const auto& path = entry.path();

            // Type filter
            if constexpr (std::is_same_v<T, Mesh>)
            {
                if (path.extension() != ".obj" &&
                    path.extension() != ".FBX" &&
                    path.extension() != ".fbx")
                    continue;
            }
            else if constexpr (std::is_same_v<T, Texture>)
            {
                if (path.extension() != ".png" &&
                    path.extension() != ".jpg")
                    continue;
            }

            std::string pathStr = path.string();

            if (m_gui.Button(pathStr.c_str()))
            {
                std::filesystem::path p = pathStr;
                std::string normalized = p.generic_string();

                *m_target = m_resourceManager.CreateAsync<T>(normalized);
                m_isOpen = false;
            }
        }

        m_gui.EndPanel();
    }
}

#endif // !ASSET_PICKER
