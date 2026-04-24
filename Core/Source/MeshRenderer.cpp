#include "MeshRenderer.h"

#include "Animator.h"
#include "Camera.h"

using namespace LibMath;
using namespace Apex;
using namespace Apex::Resources;
using namespace Apex::Rendering;

Apex::Rendering::MeshRenderer::MeshRenderer(ResourceManager* resourceManager)
{
    m_mesh = resourceManager->CreateAsync<Mesh>("ApexAssets/Meshes/cube/cube.mesh");
    m_material = resourceManager->CreateAsync<Material>("Assets/Material/Default.mat");
}

void MeshRenderer::SetMesh(ResourceHandle<Mesh> mesh)
{
    m_mesh = mesh;
}

void MeshRenderer::SetMaterial(ResourceHandle<Material> material)
{
    m_material = material;
}

ResourceHandle<Mesh>& MeshRenderer::GetMesh()
{
    return m_mesh;
}

ResourceHandle<Material>& MeshRenderer::GetMaterial()
{
    return m_material;
}

std::vector<ExposedVar> MeshRenderer::GetExposedVariables()
{
    return
    {
        { "Mesh", ExposedVar::Mesh, &m_mesh },
        { "Material", ExposedVar::Material, &m_material },
        { "Cast Shadow", ExposedVar::Bool, &m_castShadow }
    };
}

std::unique_ptr<Apex::Component> Apex::Rendering::MeshRenderer::Clone()
{
    auto clone = std::make_unique<MeshRenderer>();

    clone->m_mesh = m_mesh;
    clone->m_material = m_material;
    clone->m_castShadow = m_castShadow;

    return clone;
}

void MeshRenderer::Render(RenderPassInfo passInfo)
{
    if (!m_mesh.IsReady() ||
        !m_material.IsReady() ||
        !m_material->GetShader().IsReady() ||
        (!m_castShadow && (passInfo.m_flags & ShadowPass)))
        return;
    ResourceHandle<Shader> currentShader;

    if (passInfo.m_flags & IgnoreModelShader)
    {
        currentShader = passInfo.m_shader;
        passInfo.m_shader->Use();
    }
    else
    {
        currentShader = m_material->GetShader();
        m_material->Bind();
    }

    Data::Object* owner = GetOwner();
    if (owner)
    {
        Animator* animator = owner->GetComponent<Animator>();

        if (animator) animator->SendSkeletonMatrixToGPU(currentShader);
    }

    Matrix4 model = GetOwner()->GetGlobalTransform();

    currentShader->SetUniform("uModel", model);
    if (passInfo.m_camera)
    {
        currentShader->SetUniform("uViewProj", passInfo.m_camera->GetViewProj());
        currentShader->SetUniform("uViewPos", passInfo.m_camera->GetPosition());
    }

    if (passInfo.m_flags & UseIdAsColor) 
    {
        LibMath::Vector3 color;
        color[0] = (passInfo.m_id >> 16) & 0xFF; // highest byte
        color[1] = (passInfo.m_id >> 8) & 0xFF;  // middle byte
        color[2] = passInfo.m_id & 0xFF;         // lowest byte
        currentShader->SetUniform("uColor", color / 255.f);
    }

    m_mesh->Draw();
}

void MeshRenderer::Serialize(std::ostream& out) const
{
    out << "      \"MeshRenderer\": {\n";

    out << "        \"enabled\": " << (IsEnabled() ? "true" : "false") << ",\n";

    if (m_mesh.IsValid() && m_mesh->GetPath().size())
        out << "        \"mesh\": \"" << m_mesh->GetPath() << "\",\n";

    if (m_material.IsValid() && m_material->GetPath().size())
        out << "        \"material\": \"" << m_material->GetPath() << "\",\n";

    out << "        \"castShadow\": " << (m_castShadow ? "true" : "false ") << "\n";

    out << "      }";
}