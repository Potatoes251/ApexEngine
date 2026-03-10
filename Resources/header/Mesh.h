#ifndef MESH
#define MESH

#include "IResource.h"

#include "LibMath/Vector.h"

#include <string>
#include <vector>

class Mesh : public IResource
{
public:
	Mesh() = default;
	Mesh(std::string name) : m_name(name) {}
	Mesh(Mesh const&) = delete;
	Mesh& operator=(Mesh const&) = delete;

	~Mesh();

	void LoadFromFile() override;
	void UploadToGpu() override;

	void Draw() const;

	std::vector<Apex::Rendering::Vertex>	m_vertices;
	std::vector<unsigned int>				m_indices;

	GLsizei GetIndexCount()  const { return m_indexCount; }

private:
	std::string m_name;

	GLuint m_VAO = 0;
	GLuint m_VBO = 0;
	GLuint m_EBO = 0;
	GLsizei m_indexCount = 0;
};


#endif // !MESH
