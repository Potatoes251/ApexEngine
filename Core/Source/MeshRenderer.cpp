#include "MeshRenderer.h"

#include "Mesh.h"
#include "Texture.h"
#include "Shader.h"


namespace Apex::Rendering
{
    // ── Constructor ───────────────────────────────────────────────
    MeshRenderer::MeshRenderer(IRHI* rhi) : m_rhi(rhi) {}

    void MeshRenderer::SetMesh(Apex::Resources::ResourceHandle mesh)
    {
        m_meshHandle = mesh;
    }

    void MeshRenderer::SetTexture(Apex::Resources::ResourceHandle texture)
    {
        m_textureHandle = texture;
    }
    void MeshRenderer::SetShader(Apex::Resources::ResourceHandle shader)
    {
		m_shaderHandle = shader;
    }

    void MeshRenderer::Render(const LibMath::Matrix4& view, const LibMath::Matrix4& projection,
                              const LibMath::Vector3& cameraPosition, float deltaTime)
    {
        if (!m_meshHandle.IsReady() || !m_shaderHandle.IsReady() || !m_textureHandle.IsReady())
            return;

        Mesh* mesh = static_cast<Mesh*>   (m_meshHandle.operator->());
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

        mesh->Draw();
    }
} // namespace Apex::Rendering