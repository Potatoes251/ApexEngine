#include "Mesh.h"

#include "MeshImporter.h"

using namespace Apex::Rendering;

Mesh::~Mesh()
{
	if (!m_rhi) return;
	if (m_VAO) m_rhi->DeleteVAO(m_VAO);
	if (m_VBO) m_rhi->DeleteBuffer(m_VBO);
	if (m_EBO) m_rhi->DeleteBuffer(m_EBO);
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

	// Compute AABB now while vertices are still in CPU memory
	m_boundsMin = { 1e9f,  1e9f,  1e9f };
	m_boundsMax = { -1e9f, -1e9f, -1e9f };
	for (const auto& v : m_vertices)
	{
		for (int c = 0; c < 3; ++c)
		{
			if (v.m_position[c] < m_boundsMin[c]) m_boundsMin[c] = v.m_position[c];
			if (v.m_position[c] > m_boundsMax[c]) m_boundsMax[c] = v.m_position[c];
		}
	}
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
