#ifndef MESH
#define MESH

#include "IResource.h"

#include "LibMath/Vector.h"

#include <string>
#include <vector>

#include <glad/glad.h>

struct Vertex
{
	LibMath::Vector3	m_position;
	LibMath::Vector3	m_normal;
	LibMath::Vector2	m_textureUV;
};

class Mesh : public IResource
{
public:
	Mesh(std::string name) : m_name("../../Assets/Meshes/" + name) {}
	~Mesh();
	Mesh(Mesh const&) = delete;
	Mesh& operator=(Mesh const&) = delete;

	void LoadFromFile() override;
	void UploadToGpu() override;

	void Draw() const;

	std::vector<Vertex>			m_vertices;
	std::vector<unsigned int>	m_indices;

	GLsizei GetIndexCount()  const { return m_indexCount; }
	GLuint  GetVAO()         const { return m_VAO; }

private:
	std::string m_name;

	GLuint m_VBO = 0;
	GLuint m_VAO = 0;
	GLuint m_EBO = 0;
	GLsizei m_indexCount = 0;
};


#endif // !MESH
