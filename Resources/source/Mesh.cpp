#include "Mesh.h"

#include "MeshImporter.h"

Mesh::~Mesh()
{
	if (m_VAO) glDeleteVertexArrays(1, &m_VAO);
	if (m_VBO) glDeleteBuffers(1, &m_VBO);
	if (m_EBO) glDeleteBuffers(1, &m_EBO);
}

void Mesh::LoadFromFile()
{
	MeshImporter importer;
	importer.Import(m_name, *this);
}

void Mesh::UploadToGpu()
{
	if (m_vertices.empty())
	{
		SetState(ResourceState::Failed);
		return;
	}

	// generate VAO

	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);

	// generate VBO
	glGenBuffers(1, &m_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex), m_vertices.data(), GL_STATIC_DRAW);

	// generate EBO

	glGenBuffers(1, &m_EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int), m_indices.data(), GL_STATIC_DRAW);

	// layout: position (loc 0), normal (loc 1), texCoord (loc 2)

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_position));
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_normal));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_textureUV));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);

	m_indexCount = static_cast<GLsizei>(m_indices.size());

	m_vertices.clear();
	m_vertices.shrink_to_fit();
	m_indices.clear();
	m_indices.shrink_to_fit();
}

void Mesh::Draw() const
{
	glBindVertexArray(m_VAO);
	glDrawElements(GL_TRIANGLES, GetIndexCount(), GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);
}
