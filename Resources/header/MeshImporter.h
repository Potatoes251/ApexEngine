#ifndef MESH_IMPORTER
#define MESH_IMPORTER

#include "Mesh.h"

#include <string>
#include <mutex>

#define FBXSDK_SHARED
#include <fbxsdk.h>

// to be increased after each modifictation to the bin loader
constexpr int CURRENT_VERSION = 1;

struct MeshHeader
{
	int m_magic = 0x4D455348;
	int m_version = 1;
};

bool IsBigEndian();

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

	static FbxManager* s_manager;
	static std::mutex  s_mutex;
};

#endif // !MESH_IMPORTER

