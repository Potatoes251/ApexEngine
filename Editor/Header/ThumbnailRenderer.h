#ifndef THUMBNAIL_RENDERER
#define THUMBNAIL_RENDERER

#include "RHI.h"
#include "ResourceHandle.h"
#include "ResourceManager.h"
#include "Material.h"

#include <unordered_map>
#include <filesystem>

class Model;
namespace Fs = std::filesystem;

namespace Apex::Editor
{
    class ThumbnailRenderer
    {
    public:
        explicit ThumbnailRenderer(Apex::Rendering::IRHI& rhi,
            Apex::Resources::ResourceManager& resourceManager);
        ~ThumbnailRenderer();
        ThumbnailRenderer(const ThumbnailRenderer&) = delete;
        ThumbnailRenderer& operator=(const ThumbnailRenderer&) = delete;

        uint32_t GetCached(const Fs::path& meshPath) const;
        uint32_t GetOrRender(const Fs::path& meshPath);

        // Material thumbnail (sphere with material's albedo texture)
        uint32_t GetOrRenderMaterial(const Fs::path& matPath);

        void Clear();

        static constexpr float SIZE = 128.f;

    private:
        uint32_t Render(const Fs::path& path, Apex::Resources::ResourceHandle<Model> handle);
        uint32_t RenderMaterialPreview(Apex::Resources::ResourceHandle<Apex::Rendering::Material> mat);
        void BuildShaders();
        void DestroyShaders();

        Apex::Rendering::IRHI& m_rhi;
        Apex::Resources::ResourceManager& m_resourceManager;

        std::unordered_map<std::string, Apex::Resources::ResourceHandle<Model>> m_loadingHandles;
        std::unordered_map<std::string, Apex::Resources::ResourceHandle<Apex::Rendering::Material>> m_matLoadingHandles;

        std::unordered_map<std::string, uint32_t> m_cache;

        Apex::Rendering::RHIShaderHandle m_shader = 0;
        Apex::Rendering::RHIShaderHandle m_matShader = 0;
        Apex::Resources::ResourceHandle<Model> m_sphereMesh;
    };
}

#endif