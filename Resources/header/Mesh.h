#ifndef MESH
#define MESH

#include "IResource.h"

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
	int			m_parentIndex;
	Matrix4		m_offsetMatrix;
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

	std::vector<BoneInfo>			m_bones;		//m_offsetMatrix | inverse bind pose
 	unordered_map<std::string, int> m_boneMap;
	std::vector<Animation>			m_animations;

private:
	std::string m_name;

	GLuint m_VAO = 0;
	GLuint m_VBO = 0;
	GLuint m_EBO = 0;
	GLsizei m_indexCount = 0;
};


#endif // !MESH
