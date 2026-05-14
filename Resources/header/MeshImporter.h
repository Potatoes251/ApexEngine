#ifndef MESH_IMPORTER
#define MESH_IMPORTER

#include "Mesh.h"

#include <string>
#include <mutex>

#define FBXSDK_SHARED
#include <fbxsdk.h>

#include "BinLoader.h"

struct MaterialImportData
{
	std::string m_name;             // FBX material name becomes "<name>.mat"
	std::string m_diffusePath;      // albedo / diffuse texture
	std::string m_normalPath;       // normal map
	std::string m_specularPath;     // specular / roughness map
};

class MeshImporter
{
public:
	MeshImporter() = default;
	~MeshImporter() = default;

	static void EnsureManager();

	bool Import(std::string path, Mesh& mesh, std::vector<MaterialImportData>& materials);

private:
	bool		ImportFbx(std::string path, Mesh& mesh, std::vector<MaterialImportData>& materials);
	void		ProcessNode(FbxNode* node, Mesh& mesh);
	void		ProcessMesh(FbxMesh* fbxMesh, Mesh& mesh);
	void		ExtractBones(FbxMesh* fbxMesh, Mesh& mesh);
	void		AddBoneWeight(Mesh& mesh, int vertexID, int boneID, float weight);
	void		BuildNodeMap(FbxNode* node, std::unordered_map<std::string, FbxNode*>& map);
	void		ExtractAnimations(FbxScene* scene, Mesh& mesh);
	FbxAMatrix	GetGeometryTransform(FbxNode* node);

	void ExtractSceneMaterials(FbxScene* scene, const std::string& fbxDir, std::vector<MaterialImportData>& materials);
	std::string ResolveTexturePath(FbxSurfaceMaterial* mat, const char* propName, const std::string& fbxDir);

	void NormalizeVertexWeights(Apex::Rendering::Vertex& vertex);
	void ResolveAndSortBones(FbxScene* scene, Mesh& mesh);

	static Matrix4	ConvertFBXMatrix(const FbxAMatrix& mat);

	static inline FbxManager* s_manager = nullptr;
	static inline std::mutex  s_mutex;

	std::unordered_map<int, std::vector<int>> m_ctrlPointToVertices;
};

#endif // !MESH_IMPORTER

