#include "Renderer.h"

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

        m_shader = rhi->CreateShader({ "../../Assets/Shaders/Cube.vert", "../../Assets/Shaders/Cube.frag" });
    }

    Renderer::~Renderer() 
    {
        if (m_vao != RHI_INVALID) m_rhi->DeleteVAO(m_vao);
        if (m_vbo != RHI_INVALID) m_rhi->DeleteBuffer(m_vbo);
        if (m_ebo != RHI_INVALID) m_rhi->DeleteBuffer(m_ebo);
        if (m_shader != RHI_INVALID) m_rhi->DeleteShader(m_shader);
        if (m_texture != RHI_INVALID) m_rhi->DeleteTexture(m_texture);
    }

    // ── Texture loading ───────────────────────────────────────────────
    void Renderer::LoadTexture(const std::string& path) 
    {
        int width, height, channels;
        stbi_set_flip_vertically_on_load(true);
        unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
        if (!data) 
        {
            std::cout << "Failed to load texture: " << path << std::endl;
            return;
        }

        TextureDescription desc;
        desc.m_width = width;
        desc.m_height = height;
        desc.m_channels = channels;
        desc.m_data = data;

        m_texture = m_rhi->CreateTexture(desc);
		stbi_image_free(data);
    }
    void Renderer::Render(const LibMath::Matrix4& view, const LibMath::Matrix4& projection, 
                              const LibMath::Vector3& cameraPosition, float deltaTime)
    {
		m_rhi->BindShader(m_shader);

        float angle = m_rotationSpeed * deltaTime;
        LibMath::Radian angleRad{ angle };

        LibMath::Matrix4 model = LibMath::Matrix4::createRotationY(angleRad);

        m_rhi->SetUniformMat4(m_shader, "uModel", model);
        m_rhi->SetUniformMat4(m_shader, "uView", view);
        m_rhi->SetUniformMat4(m_shader, "uProjection", projection);
        m_rhi->SetUniformVec3(m_shader, "uCameraPos", cameraPosition);
        m_rhi->BindTexture(m_texture);
		m_rhi->DrawIndexed(static_cast<GLsizei>(CUBE_INDICES.size()));

		m_rhi->SetUniformVec3(m_shader, "uLightPos", LibMath::Vector3(2.f, 3.f, 2.f));
		m_rhi->SetUniformVec3(m_shader, "uLightColor", LibMath::Vector3(1.f, 1.f, 1.f));
		m_rhi->SetUniformVec3(m_shader, "uViewPos", cameraPosition);

        if (m_texture != RHI_INVALID) 
        {
            m_rhi->BindTexture(m_texture, 0);
			m_rhi->SetUniformInt(m_shader, "uTexture", 0);
			m_rhi->SetUniformInt(m_shader, "uUseTexture", 1);
		}
        else
        {
            m_rhi->SetUniformInt(m_shader, "uUseTexture", 0);
        }

        m_rhi->BindVAO(m_vao);
        m_rhi->DrawIndexed(static_cast<GLsizei>(CUBE_INDICES.size()));
    }
} // namespace Apex::Rendering