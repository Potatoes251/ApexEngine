#ifndef MESH_IMPORTER
#define MESH_IMPORTER

#include "Mesh.h"

#include <string>
#include <mutex>

#define FBXSDK_SHARED
#include <fbxsdk.h>

#include "BinLoader.h"

class MeshImporter
{
public:
	MeshImporter() = default;
	~MeshImporter() = default;

	static void EnsureManager();

	bool Import(std::string path, Mesh& mesh);

private:
	bool ImportFbx(std::string path, Mesh& mesh);
	void ProcessNode(FbxNode* node, Mesh& mesh);
	void ProcessMesh(FbxMesh* fbxMesh, Mesh& mesh);
	void				ExtractBones(FbxMesh* fbxMesh, Mesh& mesh);
	void				AddBoneWeight(Mesh& mesh, int vertexID, int boneID, float weight);
	void				BuildNodeMap(FbxNode* node, std::unordered_map<std::string, FbxNode*>& map);
	void				ExtractAnimations(FbxScene* scene, Mesh& mesh);
	FbxAMatrix			GetGeometryTransform(FbxNode* node);

	void NormalizeVertexWeights(Apex::Rendering::Vertex& vertex);
	void ResolveAndSortBones(FbxScene* scene, Mesh& mesh);
	Matrix4	ConvertFBXMatrix(const FbxAMatrix& mat);


	static inline FbxManager* s_manager = nullptr;
	static inline std::mutex  s_mutex;

	std::unordered_map<int, std::vector<int>> m_ctrlPointToVertices;
};

#endif // !MESH_IMPORTER

