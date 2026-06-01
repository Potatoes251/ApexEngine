#ifndef MODEL
#define MODEL

#include "IResource.h"

#include "Mesh.h"

#include "LibMath/Matrix/Matrix4.h"
#include "LibMath/Vector/Vector3.h"
#include "LibMath/Quaternion.h"

#include <unordered_map>
#include <vector>
#include <string>

struct BoneInfo
{
	LibMath::Matrix4		m_offsetMatrix;
	int32_t					m_parentIndex = -1;
	std::vector<int32_t>	m_children;
};

struct KeyFrame
{
public:
	//default local
	LibMath::Vector3	m_position;
	LibMath::Quaternion	m_rotation;
	LibMath::Vector3	m_scale;

	//global
	LibMath::Matrix4		m_matrix;
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

class Model : public IResource
{
public:
	Model() = default;
	Model(std::string path) : IResource(path) {}
	Model(Model const&) = delete;
	Model& operator=(Model const&) = delete;

	void LoadFromFile() override;
	void UploadToGpu() override;

	// Axis-aligned bounding box - populated during LoadFromFile()
	LibMath::Vector3 GetBoundsMin() const { return m_boundsMin; }
	LibMath::Vector3 GetBoundsMax() const { return m_boundsMax; }

	std::vector<Mesh> const& GetMeshes() { return m_meshes; }

	int GetAnimationIdx(std::string const& name);

	std::vector<Animation> const& GetAnimations() { return m_animations; }

	std::vector<BoneInfo>	m_skeleton;
	std::unordered_map<std::string, int>	m_boneMap;
private:
	friend class MeshImporter;
	friend class BinLoader;

	std::vector<Animation>	m_animations;
	std::vector<Mesh>	m_meshes;

	LibMath::Vector3 m_boundsMin = { FLT_MAX,  FLT_MAX,  FLT_MAX };
	LibMath::Vector3 m_boundsMax = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
};

#endif // !MODEL