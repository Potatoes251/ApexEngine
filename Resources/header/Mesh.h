#ifndef MESH
#define MESH

#include "IResource.h"
#include "RHI.h"

#include "LibMath/Vector.h"
#include "LibMath/Matrix/Matrix4.h"
#include "LibMath/Vector/Vector3.h"
#include "LibMath/Quaternion.h"


#include <string>
#include <vector>
#include <unordered_map>

using LibMath::Matrix4;
using LibMath::Vector3;
using LibMath::Quaternion;
using std::unordered_map;

struct BoneInfo
{
	Matrix4					m_offsetMatrix;
	int32_t					m_parentIndex = -1;
	std::vector<int32_t>	m_children;
};

struct KeyFrame
{
public:
	//default local
	Vector3		m_position;
	Quaternion	m_rotation;
	Vector3		m_scale;

	//global
	Matrix4		m_matrix;
};

struct SingleBoneAnimation
{
	std::vector<KeyFrame>	m_frames;
};

struct Animation
{
	std::vector<SingleBoneAnimation>	m_boneAnimations;
	std::string							m_name;
};

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
	void DrawLine() const;

	uint32_t GetIndexCount()  const { return m_indexCount; }
	uint32_t GetVertexCount()  const { return m_vertexCount; }

	// Axis-aligned bounding box - populated during UploadToGpu(), valid after IsReady()
	LibMath::Vector3 GetBoundsMin() const { return m_boundsMin; }
	LibMath::Vector3 GetBoundsMax() const { return m_boundsMax; }

	int GetAnimationIdx(std::string const& name);

	std::vector<Apex::Rendering::Vertex>	m_vertices;
	std::vector<uint32_t>					m_indices;

	std::vector<BoneInfo>					m_bones;		//with m_offsetMatrix = inverse bind pose
	std::vector<Animation>					m_animations;
private:
	friend class MeshImporter;
	friend class BinLoader;

 	unordered_map<std::string, int>	m_boneMap;
	std::vector<std::string>		m_boneNames;

	uint32_t	m_VAO = 0;
	uint32_t	m_VBO = 0;
	uint32_t	m_EBO = 0;
	uint32_t	m_indexCount = 0;
	uint32_t	m_vertexCount = 0;
	std::string m_name;

	LibMath::Vector3 m_boundsMin = { 1e9f,  1e9f,  1e9f };
	LibMath::Vector3 m_boundsMax = { -1e9f, -1e9f, -1e9f };
};


#endif // !MESH	
