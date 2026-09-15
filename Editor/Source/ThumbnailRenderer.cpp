#include "ThumbnailRenderer.h"
#include "Mesh.h"
#include "Model.h"
#include "Texture.h"

#include "LibMath/Angle.h"
#include "LibMath/Arithmetic.h"
#include "LibMath/Trigonometry.h"

namespace Apex::Editor
{

    ThumbnailRenderer::ThumbnailRenderer(Apex::Rendering::IRHI& rhi,
        Apex::Resources::ResourceManager& resourceManager)
        : m_rhi(rhi), m_resourceManager(resourceManager)
    {
        BuildShaders();
        m_sphereMesh = m_resourceManager.CreateAsync<Model>("ApexAssets/Meshes/Sphere/Sphere.mesh");
    }

    ThumbnailRenderer::~ThumbnailRenderer()
    {
        Clear();
        DestroyShaders();
    }

    uint32_t ThumbnailRenderer::GetOrRender(const Fs::path& meshPath)
    {
        std::string key = meshPath.generic_string();

        auto it = m_cache.find(key);
        if (it != m_cache.end())
            return it->second;

        auto loadIt = m_loadingHandles.find(key);
        if (loadIt == m_loadingHandles.end())
        {
            m_loadingHandles[key] = m_resourceManager.CreateAsync<Model>(key);
            return 0;
        }

        auto& handle = loadIt->second;
        if (handle.IsReady())
        {
            uint32_t textureID = Render(meshPath, handle);
            m_cache[key] = textureID;

            m_loadingHandles.erase(key);
            return textureID;
        }

        return 0;
    }

    uint32_t ThumbnailRenderer::GetCached(const Fs::path& meshPath) const
    {
        auto it = m_cache.find(meshPath.generic_string());
        return (it != m_cache.end()) ? it->second : 0;
    }

    uint32_t ThumbnailRenderer::Render(const Fs::path& path, Apex::Resources::ResourceHandle<Model> handle)
    {
        if (m_shader == 0 || !handle.IsReady()) return 0;

        Apex::Rendering::RHITextureHandle colorTexture = 0;
        Apex::Rendering::RHIFrameBufferHandle fbo = m_rhi.CreateFrameBuffer(SIZE, SIZE, colorTexture);

        m_rhi.BindFrameBuffer(fbo, SIZE, SIZE);
        m_rhi.SetClearColor(0.12f, 0.12f, 0.14f, 1.f);
        m_rhi.Clear();

        LibMath::Vector3 boundsMin = handle->GetBoundsMin();
        LibMath::Vector3 boundsMax = handle->GetBoundsMax();

        LibMath::Vector3 center = 
        {
            (boundsMin[0] + boundsMax[0]) * 0.5f,
            (boundsMin[1] + boundsMax[1]) * 0.5f,
            (boundsMin[2] + boundsMax[2]) * 0.5f 
        };

        float dx = boundsMax[0] - boundsMin[0], dy = boundsMax[1] - boundsMin[1], dz = boundsMax[2] - boundsMin[2];
        float radius = LibMath::squareRoot(dx * dx + dy * dy + dz * dz) * 0.5f;
        if (radius < 1e-4f) radius = 1.f;

        float dist = radius * 2.2f;
        LibMath::Radian yaw = LibMath::Degree{ 45.f };
        LibMath::Radian pitch = LibMath::Degree{ 25.f };

        LibMath::Vector3 eye = 
        {
            center[0] + dist * LibMath::cos(yaw) * LibMath::sin(yaw),
            center[1] + dist * LibMath::sin(pitch),
            center[2] + dist * LibMath::cos(pitch) * LibMath::cos(yaw)
        };

        LibMath::Vector3 up = { 0.f, 1.f, 0.f };
        LibMath::Matrix4 view = LibMath::Matrix4::lookAt(eye, center, up);

        float nearP = dist * 0.001f;
        float farP = dist * 100.f;
        LibMath::Degree fov = LibMath::Degree(45.f);

        LibMath::Matrix4 proj = LibMath::Matrix4::perspective(fov, 1, nearP, farP);

        LibMath::Matrix4 mvp = proj * view;
        LibMath::Matrix4 model = LibMath::Matrix4::identity();

        LibMath::Vector3 lightDir = { 0.577f, 0.577f, 0.577f };

        m_rhi.BindShader(m_shader);
        m_rhi.SetUniformMat4(m_shader, "uMVP", mvp);
        m_rhi.SetUniformMat4(m_shader, "uModel", model);
        m_rhi.SetUniformVec3(m_shader, "uLightDir", lightDir);
        m_rhi.SetUniformVec3(m_shader, "uCamPos", eye);
        m_rhi.SetUniformFloat(m_shader, "uExposure", 1.f);
        m_rhi.SetUniformVec3(m_shader, "uBaseColor", { 0.75f, 0.75f, 0.75f });

        for (Mesh const& mesh : handle->GetMeshes())
        {
            mesh.Draw();
        }

        m_rhi.UnBindFrameBuffer();

        return m_rhi.GetTexture(colorTexture);
    }

    uint32_t ThumbnailRenderer::RenderMaterialPreview(Apex::Resources::ResourceHandle<Apex::Rendering::Material> mat)
    {
        if (!m_sphereMesh.IsReady() || m_sphereMesh->GetMeshes().size() == 0) return 0;
        if (m_matShader == 0 || !mat.IsValid()) return 0;

        Apex::Rendering::RHITextureHandle colorTex = 0;
        Apex::Rendering::RHIFrameBufferHandle fbo =
            m_rhi.CreateFrameBuffer(static_cast<int>(SIZE), static_cast<int>(SIZE), colorTex);

        m_rhi.BindFrameBuffer(fbo, static_cast<int>(SIZE), static_cast<int>(SIZE));
        m_rhi.SetClearColor(0.12f, 0.12f, 0.14f, 1.f);
        m_rhi.Clear();

        // Fixed camera looking at the origin Ã¢ÂÂ sphere is at origin, radius 1
        LibMath::Vector3 eye = { 1.8f, 1.2f, 2.2f };
        LibMath::Vector3 center = { 0.f, 0.f, 0.f };
        LibMath::Vector3 up = { 0.f, 1.f, 0.f };
        LibMath::Matrix4 view = LibMath::Matrix4::lookAt(eye, center, up);
        LibMath::Matrix4 proj = LibMath::Matrix4::perspective(LibMath::Degree{ 45.f }, 1.f, 0.01f, 100.f);
        LibMath::Matrix4 mvp = proj * view;
        LibMath::Matrix4 model = LibMath::Matrix4::identity();

        LibMath::Vector3 lightDir = { 0.577f, 0.577f, 0.577f };

        m_rhi.BindShader(m_matShader);
        m_rhi.SetUniformMat4(m_matShader, "uMVP", mvp);
        m_rhi.SetUniformMat4(m_matShader, "uModel", model);
        m_rhi.SetUniformVec3(m_matShader, "uLightDir", lightDir);
        m_rhi.SetUniformVec3(m_matShader, "uCamPos", eye);
        m_rhi.SetUniformFloat(m_matShader, "uExposure", 1.f);
        m_rhi.SetUniformInt(m_matShader, "uAlbedoTex", 0);

        auto& textures = mat->GetTextures();
        bool hasTexture = !textures.empty() && textures[0].IsReady();
        if (hasTexture)
        {
            textures[0]->Bind(0);
            m_rhi.SetUniformInt(m_matShader, "uAlbedoTex", 0);
            m_rhi.SetUniformInt(m_matShader, "uUseTexture", 1);
        }
        else
        {
            m_rhi.SetUniformVec3(m_matShader, "uBaseColor", { 0.7f, 0.7f, 0.7f });
            m_rhi.SetUniformInt(m_matShader, "uUseTexture", 0);
        }

        LibMath::Vector3 tint = mat->GetTint();
        m_rhi.SetUniformVec3(m_matShader, "uTint", tint);

        m_sphereMesh->GetMeshes()[0].Draw();
        m_rhi.UnBindFrameBuffer();

        return m_rhi.GetTexture(colorTex);
    }

    uint32_t ThumbnailRenderer::GetOrRenderMaterial(const Fs::path& matPath)
    {
        std::string key = matPath.generic_string();

        auto it = m_cache.find(key);
        if (it != m_cache.end()) return it->second;

        auto loadIt = m_matLoadingHandles.find(key);
        if (loadIt == m_matLoadingHandles.end())
        {
            m_matLoadingHandles[key] = m_resourceManager.CreateAsync<Apex::Rendering::Material>(key);
            return 0;
        }

        auto& handle = loadIt->second;
        if (!handle.IsReady()) return 0;

        if (!m_sphereMesh.IsReady()) return 0;

        uint32_t id = RenderMaterialPreview(handle);
        if (id != 0)
        {
            m_cache[key] = id;
            m_matLoadingHandles.erase(key);
        }
        return id;
    }

    void ThumbnailRenderer::Clear()
    {
        m_loadingHandles.clear();
        m_cache.clear();
    }

    void ThumbnailRenderer::BuildShaders()
    {
        m_shader = m_rhi.CreateShader({
            "ApexAssets/Shaders/MeshViewerLitVert.glsl",
            "ApexAssets/Shaders/MeshViewerLitFrag.glsl"
            });
        m_matShader = m_shader;
    }

    void ThumbnailRenderer::DestroyShaders()
    {
        if (m_shader) 
        { 
            m_rhi.DeleteShader(m_shader); 
            m_shader = 0; 
        }
        if (m_matShader)
        {
            m_rhi.DeleteShader(m_matShader); 
            m_matShader = 0;
        }
    }

}