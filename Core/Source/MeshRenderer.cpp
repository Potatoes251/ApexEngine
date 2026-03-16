#include "MeshRenderer.h"

#include "Camera.h"

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

void Apex::Rendering::MeshRenderer::Render(RenderPassInfo passInfo)
{
    ResourceHandle<Shader> currentShader = m_shader;
    if (passInfo.m_useModelShader) m_shader->Use();
    else currentShader = passInfo.m_shader;


    Matrix4 model = GetOwner()->GetGlobalTransform();

    currentShader->SetUniform("uModel", model);
    currentShader->SetUniform("uViewProj", passInfo.m_camera->GetProjectionMatrix(passInfo.m_aspectRatio) * passInfo.m_camera->GetViewMatrix());
    
    currentShader->SetUniform("uLightPos", { 2.f, 3.f, 2.f });
    currentShader->SetUniform("uLightColor", { 1.f, 1.f, 1.f });
    currentShader->SetUniform("uViewPos", passInfo.m_camera->GetPosition());

    if (passInfo.m_useIdAsColor) 
    {
        LibMath::Vector3 color;
        color[0] = (passInfo.m_id >> 16) & 0xFF; // highest byte
        color[1] = (passInfo.m_id >> 8) & 0xFF;  // middle byte
        color[2] = passInfo.m_id & 0xFF;         // lowest byte
        currentShader->SetUniform("uColor", color / 255.f);
    }

    m_texture->Bind(0);
    currentShader->SetUniform("uTexture", 0);
    currentShader->SetUniform("uUseTexture", 1);
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