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
	Mesh(std::string path) : IResource(path) {}
	Mesh(Mesh const&) = delete;
	Mesh& operator=(Mesh const&) = delete;

	~Mesh();

	void LoadFromFile() override;
	void UploadToGpu() override;

	void ClearVertices();

	void Draw() const;
	void DrawLineLoop() const;

	uint32_t GetIndexCount()  const { return m_indexCount; }
	uint32_t GetVertexCount()  const { return m_vertexCount; }

	// Axis-aligned bounding box — populated during UploadToGpu(), valid after IsReady()
	LibMath::Vector3 GetBoundsMin() const { return m_boundsMin; }
	LibMath::Vector3 GetBoundsMax() const { return m_boundsMax; }

	std::vector<Apex::Rendering::Vertex>	m_vertices;
	std::vector<unsigned int>				m_indices;

private:

	uint32_t m_VAO = 0;
	uint32_t m_VBO = 0;
	uint32_t m_EBO = 0;
	uint32_t m_indexCount = 0;
	uint32_t m_vertexCount = 0;

	LibMath::Vector3 m_boundsMin = { 1e9f,  1e9f,  1e9f };
	LibMath::Vector3 m_boundsMax = { -1e9f, -1e9f, -1e9f };
};


#endif // !MESH	
