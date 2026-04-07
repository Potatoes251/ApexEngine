#ifndef MESH_IMPORTER
#define MESH_IMPORTER

#include "Mesh.h"

#include <string>
#include <mutex>

#define FBXSDK_SHARED
#include <fbxsdk.h>
#include <bit>

// to be increased after each modifictation to the bin loader
constexpr int CURRENT_VERSION = 1;

struct MeshHeader
{
	int m_magic = 0x4D455348;
	int m_version = 1;
};


class MeshImporter
{
public:
	MeshImporter() = default;
	~MeshImporter() = default;

	void Import(std::string path, Mesh& mesh);
	bool ImportBin(std::string path, Mesh& mesh);
	bool ExportBin(std::string path, Mesh& mesh);
	bool ImportFbx(std::string path, Mesh& mesh);
	void ProcessNode(FbxNode* node, Mesh& mesh);
	void ProcessMesh(FbxMesh* fbxMesh, Mesh& mesh);
	void				ExtractBones(FbxMesh* fbxMesh, Mesh& mesh);
	void				AddBoneWeight(Mesh& mesh, int vertexID, int boneID, float weight);
	void				ExtractAnimations(FbxScene* scene, Mesh& mesh);
	const std::string&	GetBoneNameFromIndex(const Mesh& mesh, int index);
	FbxAMatrix			GetGeometryTransform(FbxNode* node);

	Matrix4	ConvertFBXMatrix(const FbxAMatrix& mat);

	static constexpr bool IsBigEndian()
	{
		return std::endian::native == std::endian::big;
	}

	static FbxManager* s_manager;
	static std::mutex  s_mutex;

	std::unordered_map<int, std::vector<int>> m_ctrlPointToVertices;
};

#endif // !MESH_IMPORTER

