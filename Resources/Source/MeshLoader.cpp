#include "MeshLoader.h"

#include "BinLoader.h"
#include "MeshImporter.h"

#include "Log.H"

#include <filesystem>

void MeshLoader::Load(std::string path, Mesh& mesh)
{
    std::filesystem::path filePath(path);
    std::string extension = filePath.extension().string();

    BinLoader binLoader;
    std::string sourcePath;
    if (IsBinFile(extension) && binLoader.Load(path, mesh, sourcePath)) return;

    LOG_WARNING_CAT("Resource", "Failed to load {} from binary", path);

    if (!sourcePath.empty()) path = sourcePath;

    MeshImporter importer;
    if (importer.Import(path, mesh))
    {
        binLoader.Export(path, mesh);
        return;
    }
}

void MeshLoader::Load(std::string path, Animation& animation)
{
    std::filesystem::path filePath(path);
    std::string extension = filePath.extension().string();

    BinLoader binLoader;
    std::string sourcePath;
    if (IsBinFile(extension) && binLoader.Load(path, animation)) return;
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