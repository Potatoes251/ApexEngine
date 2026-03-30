#include "ThumbnailRenderer.h"
#include "Mesh.h"

#include <glad/glad.h>

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
    }

    ThumbnailRenderer::~ThumbnailRenderer()
    {
        Clear();
        DestroyShaders();
    }

    uint32_t ThumbnailRenderer::GetOrRender(const fs::path& meshPath)
    {
        std::string key = meshPath.generic_string();

        auto it = m_cache.find(key);
        if (it != m_cache.end())
            return it->second;

        auto loadIt = m_loadingHandles.find(key);
        if (loadIt == m_loadingHandles.end())
        {
            m_loadingHandles[key] = m_resourceManager.CreateAsync<Mesh>(key);
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

    uint32_t ThumbnailRenderer::GetCached(const fs::path& meshPath) const
    {
        auto it = m_cache.find(meshPath.generic_string());
        return (it != m_cache.end()) ? it->second : 0;
    }

    uint32_t ThumbnailRenderer::Render(const fs::path& path, Apex::Resources::ResourceHandle<Mesh> handle)
    {
        if (m_shader == 0 || !handle.IsReady()) return 0;

        Apex::Rendering::RHITextureHandle colorTexture = 0;
        Apex::Rendering::RHIFrameBufferHandle fbo = m_rhi.CreateFrameBuffer(SIZE, SIZE, colorTexture);

        m_rhi.BindFrameBuffer(fbo, SIZE, SIZE);
        glClearColor(0.12f, 0.12f, 0.14f, 1.f);
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
        float fov = LibMath::Degree{ 45.f }.radian();

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

        handle->Draw();

        m_rhi.UnBindFrameBuffer();

        return m_rhi.GetTexture(colorTexture);
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
    }

    void ThumbnailRenderer::DestroyShaders()
    {
        if (m_shader) 
        { 
            m_rhi.DeleteShader(m_shader); 
            m_shader = 0; 
        }
    }

}