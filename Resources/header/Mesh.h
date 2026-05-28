#ifndef MESH
#define MESH

#include "IResource.h"

#include "RHI.h"
#include "SSBO.h"

#include "LibMath/Vector/Vector3.h"

#include "Model.h"

#include <string>
#include <vector>
#include <unordered_map>

using std::unordered_map;

class Mesh : public IResource
{
public:
	Mesh() = default;
	Mesh(std::string path) : IResource(path) {}
	Mesh(Mesh const&) = delete;
	Mesh& operator=(Mesh const&) = delete;
	Mesh(Mesh&& other) noexcept;
	Mesh& operator=(Mesh&& other) noexcept;

	~Mesh();

	void LoadFromFile() override;
	void UploadToGpu() override;

	void ClearVertices();

	void BindForCompute() const;

	void Draw(bool skinned = false) const;
	void DrawLine() const;

	uint32_t GetIndexCount()  const { return m_indexCount; }
	uint32_t GetVertexCount()  const { return m_vertexCount; }

	// Axis-aligned bounding box - populated during UploadToGpu(), valid after IsReady()
	LibMath::Vector3 GetBoundsMin() const { return m_boundsMin; }
	LibMath::Vector3 GetBoundsMax() const { return m_boundsMax; }

	void SetBounds();

	int GetAnimationIdx(std::string const& name);

	std::vector<Apex::Rendering::Vertex>	m_vertices;
	std::vector<uint32_t>					m_indices;

	std::vector<BoneInfo>					m_bones;		//with m_offsetMatrix = inverse bind pose
	std::vector<Animation>					m_animations;
private:
	friend class MeshImporter;
	friend class BinLoader;

 	unordered_map<std::string, int>	m_boneMap;

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

	LibMath::Vector3 m_boundsMin = { 1e9f,  1e9f,  1e9f };
	LibMath::Vector3 m_boundsMax = { -1e9f, -1e9f, -1e9f };
};


#endif // !MESH	
