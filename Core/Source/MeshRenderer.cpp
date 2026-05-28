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

void MeshRenderer::ComputePass()
{
    if (!m_mesh.IsReady() ||
        !m_material.IsReady())
        return;

    ResourceHandle<ComputeShader> computeShader = m_material->GetComputeShader();

    if (!computeShader.IsReady()) 
    {
        m_useSkinnedVAO = false;
        return;
    }
    m_useSkinnedVAO = true;

    m_material->BindCompute();

    m_mesh->BindForCompute();

    Data::Object* owner = GetOwner();
    if (owner)
    {
        Animator* animator = owner->GetComponent<Animator>();

        if (animator) animator->SendSkeletonMatrixToGPU(computeShader);
    }

    m_material->StartCompute(m_mesh->GetVertexCount());
}

void MeshRenderer::Render(RenderPassInfo passInfo)
{
    if (!m_mesh.IsReady() ||
        !m_material.IsReady() ||
        (!m_castShadow && (passInfo.m_flags & ShadowPass)))
        return;

    if (!(passInfo.m_flags & IgnoreMaterial))
    {
        m_material->Bind(passInfo.m_shader);
    }

    Matrix4 model = GetOwner()->GetGlobalTransform();

    passInfo.m_shader->SetUniform("uModel", model);
    if (passInfo.m_camera)
    {
        passInfo.m_shader->SetUniform("uViewProj", passInfo.m_camera->GetViewProj());
        passInfo.m_shader->SetUniform("uView", passInfo.m_camera->GetViewMatrix());
        passInfo.m_shader->SetUniform("uViewPos", passInfo.m_camera->GetPosition());
    }

    if (passInfo.m_flags & UseIdAsColor)
    {
        size_t id = GetOwner()->GetId();
        LibMath::Vector3 color;
        color[0] = (id >> 16) & 0xFF; // highest byte
        color[1] = (id >> 8) & 0xFF;  // middle byte
        color[2] = id & 0xFF;         // lowest byte
        passInfo.m_shader->SetUniform("uColor", color / 255.f);
    }

    m_mesh->Draw(m_useSkinnedVAO);
}

void MeshRenderer::Serialize(std::ostream& out) const
{
    if (m_mesh.IsValid() && !m_mesh->GetPath().empty())
        out << "        \"mesh\": \"" << m_mesh->GetPath() << "\",\n";

    if (m_material.IsValid() && !m_material->GetPath().empty())
        out << "        \"material\": \"" << m_material->GetPath() << "\",\n";

    out << "        \"castShadow\": " << (m_castShadow ? "true" : "false ") << "\n";
}