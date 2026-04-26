#include "Mesh.h"

#include "MeshLoader.h"

using namespace Apex::Rendering;

Mesh::~Mesh()
{
	if (m_VAO) m_rhi->DeleteVAO(m_VAO);
	if (m_VBO) m_rhi->DeleteBuffer(m_VBO);
	if (m_EBO) m_rhi->DeleteBuffer(m_EBO);
}

void Mesh::LoadFromFile()
{
	MeshLoader loader;
	loader.Load(m_path, *this);

	for (const auto& v : m_vertices)
	{
		for (int c = 0; c < 3; ++c)
		{
			if (v.m_position[c] < m_boundsMin[c]) m_boundsMin[c] = v.m_position[c];
			if (v.m_position[c] > m_boundsMax[c]) m_boundsMax[c] = v.m_position[c];
		}
	}
}

void Mesh::UploadToGpu()
{
	if (m_vertices.empty())
	{
		SetState(ResourceState::Failed);
		return;
	}

	m_vertexCount = static_cast<uint32_t>(m_vertices.size());
	m_indexCount = static_cast<uint32_t>(m_indices.size());

	m_VAO = m_rhi->CreateVAO();
	m_rhi->BindVAO(m_VAO);

	m_VBO = m_rhi->CreateBuffer(BufferType::Vertex, m_vertices.data(), m_vertexCount * sizeof(Vertex));
	m_EBO = m_rhi->CreateBuffer(BufferType::Index, m_indices.data(), m_indexCount * sizeof(int));

	m_rhi->SetupVertexLayout();
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

void Mesh::DrawLine() const
{
	m_rhi->BindVAO(m_VAO);
	m_rhi->DrawLineArrays(m_vertexCount);
}

int Mesh::GetAnimationIdx(std::string const& name)
{
	for (int i = 0; i < m_animations.size(); i++)
	{
		Animation const& anim = m_animations[i];
		if (anim.m_name == name)
			return i;
	}
	return -1;
}
