#include "Skybox.h"


using namespace Apex::Rendering;

Skybox::Skybox(Resources::ResourceHandle<Cubemap> cubemap, Resources::ResourceHandle<Mesh> mesh, 
	Resources::ResourceHandle<Shader> shader, IRHI* rhi) 
	: m_cubemap(cubemap), m_mesh(mesh), m_shader(shader), m_rhi(rhi)
{}

Skybox::Skybox(Skybox const& other)
{
	m_cubemap = other.m_cubemap;
	m_mesh = other.m_mesh;
	m_shader = other.m_shader;
}

Skybox& Skybox::operator=(Skybox const& other)
{
	m_cubemap = other.m_cubemap;
	m_mesh = other.m_mesh;
	m_shader = other.m_shader;
	return *this;
}

void Skybox::Render(LibMath::Matrix4 viewProj)
{
	if (!m_cubemap.IsReady() || !m_shader.IsReady() || !m_mesh.IsReady()) return;

	//m_rhi->SetDepthTest(false)
	m_shader->Use();
	m_shader->SetUniform("viewProjection", viewProj);

	m_cubemap->Bind();

	m_mesh->Draw();
}