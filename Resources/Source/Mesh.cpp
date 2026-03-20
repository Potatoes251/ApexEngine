#include "Mesh.h"

#include "MeshImporter.h"

using namespace Apex::Rendering;

Mesh::~Mesh()
{
	if (m_rhi)
	{
		m_rhi->DeleteVAO(m_VAO);
		m_rhi->DeleteBuffer(m_VBO);
		m_rhi->DeleteBuffer(m_EBO);
	}
}

void Mesh::LoadFromFile()
{
	MeshImporter importer;
	importer.Import(m_path, *this);
}

void Mesh::UploadToGpu()
{
	if (m_vertices.empty())
	{
		SetState(ResourceState::Failed);
		return;
	}

	m_VAO = m_rhi->CreateVAO();
	m_rhi->BindVAO(m_VAO);

	m_VBO = m_rhi->CreateBuffer(BufferType::Vertex, m_vertices.data(), m_vertices.size() * sizeof(Vertex));
	m_EBO = m_rhi->CreateBuffer(BufferType::Index, m_indices.data(), m_indices.size() * sizeof(int));

	m_rhi->SetupVertexLayout();

	m_indexCount = static_cast<uint32_t>(m_indices.size());
	m_vertexCount = static_cast<uint32_t>(m_vertices.size());
}

void Mesh::ClearVertices()
{
	m_vertices.clear();
	m_vertices.shrink_to_fit();
	m_indices.clear();
	m_indices.shrink_to_fit();
}

void Mesh::Draw() const
{
	m_rhi->BindVAO(m_VAO);
	m_rhi->DrawIndexed(m_indexCount);
}

void Mesh::DrawLineLoop() const
{
	m_rhi->BindVAO(m_VAO);
	m_rhi->DrawLineLoop(m_vertexCount);
}
