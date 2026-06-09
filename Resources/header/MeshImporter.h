#ifndef MESH_IMPORTER
#define MESH_IMPORTER

#include <string>
#include <mutex>

#define FBXSDK_SHARED

#include <fbxsdk.h>

#include "BinLoader.h"

class Mesh;
class Model;

struct MaterialImportData
{
	std::string m_name;             // FBX material name becomes "<name>.mat"
	std::string m_diffusePath;      // albedo / diffuse texture
	std::string m_normalPath;       // normal map
	std::string m_specularPath;     // specular / roughness map
	LibMath::Vector3 m_tint = { 1, 1, 1};
};

class MeshImporter
{
public:
	MeshImporter() = default;
	~MeshImporter() = default;

	static void EnsureManager();

	bool Import(std::string const& path, Model& model, std::vector<MaterialImportData>& materials);

private:
	bool		ImportFbx(std::string const& path, Model& model, std::vector<MaterialImportData>& materials);
	void		ProcessNode(FbxNode* node, Model& model);
	void		ProcessMesh(FbxMesh* fbxMesh, Mesh& mesh);
	void		ExtractBones(FbxMesh* fbxMesh, Model& model, Mesh& mesh);
	void		AddBoneWeight(Mesh& mesh, int vertexID, int boneID, float weight);
	void		BuildNodeMap(FbxNode* node, std::unordered_map<std::string, FbxNode*>& map);
	void		ExtractAnimations(FbxScene* scene, Model& model);
	FbxAMatrix	GetGeometryTransform(FbxNode* node);

	void ExtractSceneMaterials(FbxScene* scene, const std::string& fbxDir, std::vector<MaterialImportData>& materials);
	std::string ResolveTexturePath(FbxSurfaceMaterial* mat, const char* propName, const std::string& fbxDir);

	void NormalizeVertexWeights(Apex::Rendering::Vertex& vertex);
	void ResolveAndSortBones(FbxScene* scene, Model& model);

	static LibMath::Matrix4	ConvertFBXMatrix(const FbxAMatrix& mat);

	static inline FbxManager* s_manager = nullptr;
	static inline std::mutex  s_mutex;

	std::unordered_map<int, std::vector<int>> m_ctrlPointToVertices;
	std::vector<std::string>		m_boneNames;
};

#endif // !MESH_IMPORTER

