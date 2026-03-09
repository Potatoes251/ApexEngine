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