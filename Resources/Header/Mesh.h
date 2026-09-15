#ifndef MESH
#define MESH

#include "RHI.h"
#include "SSBO.h"

#include "LibMath/Vector/Vector3.h"

#include <string>
#include <vector>
#include <unordered_map>

using std::unordered_map;

class Mesh
{
public:
	Mesh() = default;
	Mesh(Mesh const&) = delete;
	Mesh& operator=(Mesh const&) = delete;
	Mesh(Mesh&& other) noexcept;
	Mesh& operator=(Mesh&& other) noexcept;

	~Mesh();

	void SetRhi(Apex::Rendering::IRHI* rhi) { m_rhi = rhi; }

	void UploadToGpu();

	void ClearVertices();

	void BindForCompute() const;

	void Draw(bool skinned = false) const;
	void DrawLine() const;

	uint32_t GetIndexCount()  const { return m_indexCount; }
	uint32_t GetVertexCount()  const { return m_vertexCount; }

	std::string const& GetName() const { return m_name; }

	int GetAnimationIdx(std::string const& name);

	std::vector<Apex::Rendering::Vertex>	m_vertices;
	std::vector<uint32_t>					m_indices;

private:
	friend class MeshImporter;
	friend class BinLoader;

	Apex::Rendering::IRHI* m_rhi = nullptr;

	uint32_t	m_VAO = 0;
	// VAO used with a compute shader
	uint32_t	m_VAOSkinned = 0;
	uint32_t	m_VBO = 0;
	uint32_t	m_EBO = 0;
	uint32_t	m_indexCount = 0;
	uint32_t	m_vertexCount = 0;
	std::string m_name;

	Apex::Rendering::Ssbo m_inputVertices;
	Apex::Rendering::Ssbo m_outputVertices;
};


#endif // !MESH	
