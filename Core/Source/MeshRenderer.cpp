#include "MeshRenderer.h"

using namespace LibMath;
using namespace Apex::Resources;

void Apex::Rendering::MeshRenderer::SetMesh(ResourceHandle<Mesh>& mesh)
{
    m_mesh = mesh;
}

void Apex::Rendering::MeshRenderer::SetTexture(ResourceHandle<Texture>& texture)
{
    m_texture = texture;
}

void Apex::Rendering::MeshRenderer::SetShader(ResourceHandle<Shader>& shader)
{
    m_shader = shader;
}

Apex::Resources::ResourceHandle<Mesh>& Apex::Rendering::MeshRenderer::GetMesh()
{
    return m_mesh;
}

Apex::Resources::ResourceHandle<Texture>& Apex::Rendering::MeshRenderer::GetTexture()
{
    return m_texture;
}

Apex::Resources::ResourceHandle<Shader>& Apex::Rendering::MeshRenderer::GetShader()
{
    return m_shader;
}

void Apex::Rendering::MeshRenderer::Render(const Matrix4& view,
    const Matrix4& projection,
    const Vector3& cameraPos)
{
    m_shader->Use();

    Matrix4 model = GetOwner()->GetGlobalTransform();

    m_shader->SetUniform("uModel", model);
    m_shader->SetUniform("uView", view);
    m_shader->SetUniform("uProjection", projection);

    m_shader->SetUniform("uLightPos", { 2.f, 3.f, 2.f });
    m_shader->SetUniform("uLightColor", { 1.f, 1.f, 1.f });
    m_shader->SetUniform("uViewPos", cameraPos);

    m_texture->Bind(0);
    m_shader->SetUniform("uTexture", 0);
    m_shader->SetUniform("uUseTexture", 1);
    m_mesh->Draw();
}

void Apex::Rendering::MeshRenderer::Serialize(std::ostream& out) const
{
    out << "      \"MeshRenderer\": {\n";

    out << "        \"enabled\": " << (IsEnabled() ? "true" : "false") << ",\n";

    if (m_mesh.IsValid() && m_mesh->GetPath().size())
        out << "        \"mesh\": \"" << m_mesh->GetPath() << "\",\n";

    if (m_texture.IsValid() && m_texture->GetPath().size())
        out << "        \"texture\": \"" << m_texture->GetPath() << "\",\n";

    if (m_shader.IsValid() && m_shader->GetPath().size())
        out << "        \"shader\": \"" << m_shader->GetPath() << "\"\n";

    out << "      }";
}