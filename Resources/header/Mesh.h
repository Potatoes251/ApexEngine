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
	Mesh(std::string name) : m_name("../../assets/Meshes/" + name) {}
	Mesh(Mesh const&) = delete;
	Mesh& operator=(Mesh const&) = delete;

	//~Mesh();

	void LoadFromFile() override;
	void UploadToGpu() override;


	std::vector<Vertex>			m_vertices;
	std::vector<unsigned int>	m_indices;

private:
	std::string m_name;

	GLuint m_VBO;
	GLuint m_VAO;
	GLuint m_EBO;
};


#endif // !MESH
