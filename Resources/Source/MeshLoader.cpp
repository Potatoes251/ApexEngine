#include "MeshLoader.h"

#include "BinLoader.h"
#include "MeshImporter.h"

#include <filesystem>

void MeshLoader::Load(std::string path, Mesh& mesh)
{
    std::filesystem::path filePath(path);
    std::string extension = filePath.extension().string();

    BinLoader binLoader;
    if (IsBinFile(extension) && binLoader.Load(path, mesh)) return;

    MeshImporter importer;
    if (importer.Import(path, mesh))
    {
        binLoader.Export(path, mesh);
        return;
    }
}

void MeshLoader::Import(std::string path, Mesh& mesh)
{
    MeshImporter importer;
    if (importer.Import(path, mesh))
    {
        BinLoader binLoader;
        binLoader.Export(path, mesh);
        return;
    }
}

bool MeshLoader::IsBinFile(std::string const& extension)
{
    std::vector<std::string> binExtensions = { ".mesh", ".anim" };

    for (std::string const& ext : binExtensions)
    {
        if (ext == extension) return true;
    }
    return false;
}