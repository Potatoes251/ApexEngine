#ifndef MESH_LOADER
#define MESH_LOADER

class Model;

#include <string>

class MeshLoader
{
public:
	MeshLoader() = default;
	MeshLoader(MeshLoader const&) = delete;
	MeshLoader& operator=(MeshLoader const&) = delete;
	~MeshLoader() = default;

	// regular loading use custom files

	static void Load(std::string const& path, Model& model);
	// force skip custom file and overwrite them
	
	static void Import(std::string const& path);

private:
	static bool IsBinFile(std::string const& extension);
};


#endif // !MESH_LOADER

