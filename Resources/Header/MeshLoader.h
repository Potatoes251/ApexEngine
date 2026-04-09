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

	void Load(std::string path, Mesh& mesh);
	// force skip custom file and overwrite them
	
	void Import(std::string path, Mesh& mesh);

private:
	bool IsBinFile(std::string const& extension);
};


#endif // !MESH_LOADER

