#ifndef MESH_IMPORTER
#define MESH_IMPORTER

#include "Mesh.h"

#include <string>
#define FBXSDK_SHARED
#include <fbxsdk.h>

class MeshImporter
{
public:
	MeshImporter() = default;

	void Import(std::string path, Mesh& mesh);
	void ProcessNode(FbxNode* node, Mesh& mesh);
	void ProcessMesh(FbxMesh* fbxMesh, Mesh& mesh);
};

#endif // !MESH_IMPORTER

