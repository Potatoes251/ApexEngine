#include "MeshRenderer.h"

#include "Animator.h"
#include "Camera.h"

using namespace Apex;
using namespace Apex::Resources;
using namespace Apex::Rendering;

Apex::Rendering::MeshRenderer::MeshRenderer(ResourceManager* resourceManager)
{
    m_model = resourceManager->CreateAsync<Model>("ApexAssets/Meshes/cube/cube.mesh");
    m_materials.push_back(resourceManager->CreateAsync<Material>("Assets/Material/Default.mat"));
}

void MeshRenderer::SetModel(Apex::Resources::ResourceHandle<Model> model)
{ 
    m_model = model;
}

void MeshRenderer::SetMaterials(std::vector<Apex::Resources::ResourceHandle<Material>> materials)
{
    m_materials = materials;
}

std::vector<ExposedVar> MeshRenderer::GetExposedVariables()
{
    std::vector<ExposedVar> variables;

    variables.push_back({ "Model", ExposedVar::Mesh, &m_model });

    for (size_t i = 0; i < m_materials.size(); i++)
    {
        std::string name = "Material" + std::to_string(i);

        if (i < m_model->GetMeshes().size() && !m_model->GetMeshes()[i].GetName().empty())
        {
            name = m_model->GetMeshes()[i].GetName();
        }

        ResourceHandle<Material>& mat = m_materials[i];
        variables.push_back({ name, ExposedVar::Material, &mat});
    }

    variables.push_back({ "Cast Shadow", ExposedVar::Bool, &m_castShadow });

    return variables;
}

void MeshRenderer::ComputePass()
{
    if (!m_model.IsReady())
        return;

    m_useSkinnedVAO = false;

    Data::Object* owner = GetOwner();
    Animator* animator = nullptr;
    if (owner)
    {
        animator = owner->GetComponent<Animator>();
    }

    std::vector<Mesh> const& meshes = m_model->GetMeshes();

    for (size_t i = 0; i < meshes.size(); i++)
    {
        if (i >= m_materials.size()) break;

        ResourceHandle<Material> material = m_materials[i];

        if (!material.IsReady()) continue;
        if (!material->GetComputeShader().IsReady()) continue;
        
        m_useSkinnedVAO = true;

        material->BindCompute();

        if (animator) 
           animator->SendSkeletonMatrixToGPU(material->GetComputeShader());

        Mesh const& mesh = meshes[i];

        mesh.BindForCompute();
        material->StartCompute(mesh.GetVertexCount());
    }
}

void MeshRenderer::Render(RenderPassInfo passInfo)
{
    if (!m_model.IsReady() ||
        (!m_castShadow && (passInfo.m_flags & ShadowPass)))
        return;
    
    if (m_materials.size() < m_model->GetMeshes().size())
        m_materials.resize(m_model->GetMeshes().size());

    LibMath::Matrix4 modelMatrix = GetOwner()->GetGlobalTransform();

    passInfo.m_shader->SetUniform("uModel", modelMatrix);
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

    std::vector<Mesh> const& meshes = m_model->GetMeshes();

    //m_material->Bind(passInfo.m_shader);

    for (size_t i = 0; i < meshes.size(); i++)
    {
        if (i < m_materials.size() && m_materials[i].IsReady() && !(passInfo.m_flags & IgnoreMaterial))
        {
            m_materials[i]->Bind(passInfo.m_shader);
        }

        meshes[i].Draw(m_useSkinnedVAO);
    }
}

void MeshRenderer::Serialize(std::ostream& out) const
{
    if (m_model.IsValid() && !m_model->GetPath().empty())
        out << "        \"model\": \"" << m_model->GetPath() << "\",\n";

    out << "        \"materials\": [\n";
    for (size_t i = 0; i < m_materials.size(); ++i)
    {
        if (m_materials[i].IsValid() && !m_materials[i]->GetPath().empty())
            out << "            \"" << m_materials[i]->GetPath() << "\"";
        else
            out << "            \"\"";
    
        if (i < m_materials.size() - 1)
            out << ",";
        out << "\n";
    }
    out << "        ],\n";

    out << "        \"castShadow\": " << (m_castShadow ? "true" : "false ") << "\n";
}