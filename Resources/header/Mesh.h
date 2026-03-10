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

	int32_t GetIndexCount()  const { return m_indexCount; }

private:
	std::string m_name;

	uint32_t m_VAO = 0;
	uint32_t m_VBO = 0;
	uint32_t m_EBO = 0;
	int32_t	m_indexCount = 0;
};


#endif // !MESH
