#ifndef MESH_LOADER
#define MESH_LOADER

#include "Mesh.h"

#include <string>

class MeshLoader
{
public:
	MeshLoader() = default;
	MeshLoader(MeshLoader const&) = delete;
	MeshLoader& operator=(MeshLoader const&) = delete;
	~MeshLoader() = default;

	// regular loading use custom files

	static void Load(std::string path, Mesh& mesh);
	static void Load(std::string path, Animation& animation);
	// force skip custom file and overwrite them
	
	static void Import(std::string path);

private:
	static bool IsBinFile(std::string const& extension);
};


#endif // !MESH_LOADER

