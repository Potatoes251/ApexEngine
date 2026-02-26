#include "Renderer.h"

#include "Texture.h"
#include "Shader.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace Apex::Rendering
{
    // ── Cube geometry (24 unique vertices, 36 indices) ────────────
    static const std::vector<Vertex> CUBE_VERTS = 
    {
        // Front (+Z)
        {{-0.5f,-0.5f, 0.5f},{0,0,1},{0,0}},{{0.5f,-0.5f, 0.5f},{0,0,1},{1,0}},
        {{ 0.5f, 0.5f, 0.5f},{0,0,1},{1,1}},{{-0.5f, 0.5f, 0.5f},{0,0,1},{0,1}},
        // Back (-Z)
        {{ 0.5f,-0.5f,-0.5f},{0,0,-1},{0,0}},{{-0.5f,-0.5f,-0.5f},{0,0,-1},{1,0}},
        {{-0.5f, 0.5f,-0.5f},{0,0,-1},{1,1}},{{ 0.5f, 0.5f,-0.5f},{0,0,-1},{0,1}},
        // Left (-X)
        {{-0.5f,-0.5f,-0.5f},{-1,0,0},{0,0}},{{-0.5f,-0.5f, 0.5f},{-1,0,0},{1,0}},
        {{-0.5f, 0.5f, 0.5f},{-1,0,0},{1,1}},{{-0.5f, 0.5f,-0.5f},{-1,0,0},{0,1}},
        // Right (+X)
        {{ 0.5f,-0.5f, 0.5f},{1,0,0},{0,0}},{{ 0.5f,-0.5f,-0.5f},{1,0,0},{1,0}},
        {{ 0.5f, 0.5f,-0.5f},{1,0,0},{1,1}},{{ 0.5f, 0.5f, 0.5f},{1,0,0},{0,1}},
        // Top (+Y)
        {{-0.5f, 0.5f, 0.5f},{0,1,0},{0,0}},{{ 0.5f, 0.5f, 0.5f},{0,1,0},{1,0}},
        {{ 0.5f, 0.5f,-0.5f},{0,1,0},{1,1}},{{-0.5f, 0.5f,-0.5f},{0,1,0},{0,1}},
        // Bottom (-Y)
        {{-0.5f,-0.5f,-0.5f},{0,-1,0},{0,0}},{{ 0.5f,-0.5f,-0.5f},{0,-1,0},{1,0}},
        {{ 0.5f,-0.5f, 0.5f},{0,-1,0},{1,1}},{{-0.5f,-0.5f, 0.5f},{0,-1,0},{0,1}},
    };

    static const std::vector<int> CUBE_INDICES = 
    {
         0, 1, 2,  2, 3, 0,   // Front
         4, 5, 6,  6, 7, 4,   // Back
         8, 9,10, 10,11, 8,   // Left
        12,13,14, 14,15,12,   // Right
        16,17,18, 18,19,16,   // Top
        20,21,22, 22,23,20,   // Bottom
    };

    // ── Constructor ───────────────────────────────────────────────
    Renderer::Renderer(IRHI* rhi) : m_rhi(rhi) 
    {
        m_vao = rhi->CreateVAO();
        rhi->BindVAO(m_vao);

        m_vbo = rhi->CreateBuffer(BufferType::Vertex,
            CUBE_VERTS.data(), CUBE_VERTS.size() * sizeof(Vertex));
        m_ebo = rhi->CreateBuffer(BufferType::Index,
            CUBE_INDICES.data(), CUBE_INDICES.size() * sizeof(int));

        rhi->SetupVertexLayout();
    }

    Renderer::~Renderer() 
    {
        if (m_vao != RHI_INVALID) m_rhi->DeleteVAO(m_vao);
        if (m_vbo != RHI_INVALID) m_rhi->DeleteBuffer(m_vbo);
        if (m_ebo != RHI_INVALID) m_rhi->DeleteBuffer(m_ebo);
    }

    void Renderer::SetTexture(Apex::Resources::ResourceHandle texture)
    {
        m_textureHandle = texture;
    }
    void Renderer::SetShader(Apex::Resources::ResourceHandle shader)
    {
		m_shaderHandle = shader;
    }

    void Renderer::Render(const LibMath::Matrix4& view, const LibMath::Matrix4& projection,
                              const LibMath::Vector3& cameraPosition, float deltaTime)
    {
        if (!m_shaderHandle.IsReady() || !m_textureHandle.IsReady())
            return;
        
        Shader* shader = static_cast<Shader*>(m_shaderHandle.operator->());
        Texture* texture = static_cast<Texture*>(m_textureHandle.operator->());

        shader->Use();

        GLuint prog = shader->GetProgram();

        float angle = m_rotationSpeed * deltaTime;
        LibMath::Radian angleRad{ angle };

        LibMath::Matrix4 model = LibMath::Matrix4::createRotationY(angleRad);

        m_rhi->SetUniformMat4(prog, "uModel", model);
        m_rhi->SetUniformMat4(prog, "uView", view);
        m_rhi->SetUniformMat4(prog, "uProjection", projection);

		m_rhi->SetUniformVec3(prog, "uLightPos", LibMath::Vector3(2.f, 3.f, 2.f));
		m_rhi->SetUniformVec3(prog, "uLightColor", LibMath::Vector3(1.f, 1.f, 1.f));
		m_rhi->SetUniformVec3(prog, "uViewPos", cameraPosition);

		texture->Bind(0);
        m_rhi->SetUniformInt(prog, "uTexture", 0);
		m_rhi->SetUniformInt(prog, "uUseTexture", 1);

        m_rhi->BindVAO(m_vao);
        m_rhi->DrawIndexed(static_cast<GLsizei>(CUBE_INDICES.size()));
    }
} // namespace Apex::Rendering