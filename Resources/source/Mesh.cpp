#include "Mesh.h"

#include "MeshImporter.h"

void Mesh::LoadFromFile()
{
	MeshImporter importer;
	importer.Import(m_path, *this);
	printf("finished loading\n");
}

void Mesh::UploadToGpu()
{
	if (m_vertices.empty())
	{
		SetState(ResourceState::Failed);
		return;
	}
	// generate VBO
	glGenBuffers(1, &m_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex), m_vertices.data(), GL_STATIC_DRAW);

	// generate EBO

	glGenBuffers(1, &m_EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(uint32_t), m_indices.data(), GL_STATIC_DRAW);

	// generate VAO

	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);

	// mesh :  pos_x, pos_y, pos_z, nor_x, nor_y, nor_z, tex_u, tex_v

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);	// pos
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));	// nor
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));	// tex
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);

	printf("finished upload of %s", m_path.c_str());
}
