#ifndef MESH
#define MESH

#include "IResource.h"

#include "LibMath/Vector.h"
#include "LibMath/Matrix/Matrix4.h"

#include <string>
#include <vector>
#include <unordered_map>

using LibMath::Matrix4;
using std::unordered_map;

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

	std::vector<Matrix4>			m_bones;		//offsetMatrix | inverse bind pose
 	unordered_map<std::string, int> m_boneMap;

private:
	std::string m_name;

	GLuint m_VAO = 0;
	GLuint m_VBO = 0;
	GLuint m_EBO = 0;
	GLsizei m_indexCount = 0;
};


#endif // !MESH
