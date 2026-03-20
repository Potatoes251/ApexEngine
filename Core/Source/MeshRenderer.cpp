#include "MeshRenderer.h"

#include "Camera.h"

using namespace LibMath;
using namespace Apex;
using namespace Apex::Resources;
using namespace Apex::Rendering;

void MeshRenderer::SetMesh(ResourceHandle<Mesh>& mesh)
{
    m_mesh = mesh;
}

void MeshRenderer::SetTexture(ResourceHandle<Texture>& texture)
{
    m_texture = texture;
}

void MeshRenderer::SetShader(ResourceHandle<Shader>& shader)
{
    m_shader = shader;
}

ResourceHandle<Mesh>& MeshRenderer::GetMesh()
{
    return m_mesh;
}

ResourceHandle<Texture>& MeshRenderer::GetTexture()
{
    return m_texture;
}

ResourceHandle<Shader>& MeshRenderer::GetShader()
{
    return m_shader;
}

std::vector<ExposedVar> MeshRenderer::GetExposedVariables()
{
    return
    {
        { "Mesh", ExposedVar::Mesh, &m_mesh },
        { "Texture", ExposedVar::Texture, &m_texture }
    };
}

void Apex::Rendering::MeshRenderer::DrawVariables(Apex::UserInterface::IGUI& gui)
{
    gui.Text("Mesh: " + m_mesh->GetPath());
    gui.Text("Texture: " + m_texture->GetPath());
}

std::unique_ptr<Apex::Component> Apex::Rendering::MeshRenderer::Clone() const
{
    auto clone = std::make_unique<MeshRenderer>();
    clone->m_mesh = m_mesh;
    clone->m_shader = m_shader;
    clone->m_texture = m_texture;
    return clone;
}

void MeshRenderer::Render(RenderPassInfo passInfo)
{
    if (!m_shader.IsReady() ||
        !m_mesh.IsReady() ||
        !m_texture.IsReady())
        return;
    ResourceHandle<Shader> currentShader;
    if (passInfo.m_useModelShader)
    {
        currentShader = m_shader;
        m_shader->Use();
    }
    else
    {
        currentShader = passInfo.m_shader;
        passInfo.m_shader->Use();
    }

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

void MeshRenderer::Serialize(std::ostream& out) const
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