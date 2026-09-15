#include "Skybox.h"

#include "RHI.h"

#include "Cubemap.h"
#include "Model.h"
#include "Mesh.h"
#include "Shader.h"

using namespace Apex::Rendering;

Skybox::Skybox(Resources::ResourceHandle<Cubemap> cubemap, Resources::ResourceHandle<Model> mesh, 
	Resources::ResourceHandle<Shader> shader) 
	: m_cubemap(cubemap), m_model(mesh), m_shader(shader)
{}

Skybox::Skybox(Skybox const& other)
{
	m_cubemap = other.m_cubemap;
	m_model = other.m_model;
	m_shader = other.m_shader;
}

Skybox& Skybox::operator=(Skybox const& other)
{
	m_cubemap = other.m_cubemap;
	m_model = other.m_model;
	m_shader = other.m_shader;
	return *this;
}

void Skybox::Render(LibMath::Matrix4 viewProj)
{
	if (!m_cubemap.IsReady() || !m_shader.IsReady() || !m_model.IsReady() || m_model->GetMeshes().size() == 0) return;

	m_shader->Use();
	m_shader->SetUniform("viewProjection", viewProj);

	m_cubemap->Bind();

	m_model->GetMeshes()[0].Draw();
}