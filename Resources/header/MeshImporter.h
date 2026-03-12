#ifndef MESH_IMPORTER
#define MESH_IMPORTER

#include "Mesh.h"

#include <string>
#include <mutex>

#define FBXSDK_SHARED
#include <fbxsdk.h>

class MeshImporter
{
public:
	MeshImporter() = default;
	~MeshImporter() = default;

	void	Import(std::string path, Mesh& mesh);
	void	ProcessNode(FbxNode* node, Mesh& mesh);
	void	ProcessMesh(FbxMesh* fbxMesh, Mesh& mesh);
	void	ExtractBones(FbxMesh* fbxMesh, Mesh& mesh);
	void	AddBoneWeight(Mesh& mesh, int vertexID, int boneID, float weight);

	Matrix4	ConvertFBXMatrix(const FbxAMatrix& mat);

	static FbxManager* s_manager;
	static std::mutex  s_mutex;
};

#endif // !MESH_IMPORTER

