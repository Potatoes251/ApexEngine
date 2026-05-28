#include "MeshLoader.h"

#include "BinLoader.h"
#include "MeshImporter.h"
#include "Model.h"

#include "Log.H"

#include <filesystem>
#include <fstream>
#include <cassert>

void MeshLoader::Load(std::string const& path, Model& model)
{
    std::filesystem::path filePath(path);
    std::string extension = filePath.extension().string();

    BinLoader binLoader;
    if (IsBinFile(extension) && binLoader.Load(path, model)) return;

    LOG_WARNING_CAT("Resource", "Failed to load {} from binary", path);
}

static void WriteMaterialFile(const std::filesystem::path& fbxPath, const MaterialImportData& mat)
{
    if (mat.m_name.empty()) return;

    std::filesystem::path matPath = fbxPath.parent_path() / (mat.m_name + ".mat");

    // Don't overwrite an existing hand-authored material
    if (std::filesystem::exists(matPath)) return;

    std::ofstream file(matPath);
    if (!file) return;

    file << "{\n";
    file << "\t\"shader\": \"ApexAssets/Shaders/Mesh.vert|ApexAssets/Shaders/Mesh.frag\",\n";
    file << "\t\"tint\": [1.0, 1.0, 1.0],\n";
    file << "\t\"textures\": [";

    bool first = true;
    if (!mat.m_diffusePath.empty())
    {
        if (!first) file << ",";
        file << "\n\t\t\"" << mat.m_diffusePath << "\"";
        first = false;
    }
    if (!mat.m_normalPath.empty())
    {
        if (!first) file << ",";
        file << "\n\t\t\"" << mat.m_normalPath << "\"";
        first = false;
    }
    if (!mat.m_specularPath.empty())
    {
        if (!first) file << ",";
        file << "\n\t\t\"" << mat.m_specularPath << "\"";
    }
    if (first)
    {
        // No textures — write a blank texture so Bind() never crashes
        file << "\n\t\t\"Assets/Textures/Blank.png\"";
    }

    file << "\n\t]\n}\n";

    LOG_INFO_CAT("Resource", "Generated material file: {}", matPath.generic_string());
}

void MeshLoader::Import(std::string const& path)
{
    Model model;
    std::vector<MaterialImportData> materials;
    MeshImporter importer;

    if (importer.Import(path, model, materials))
    {
        BinLoader binLoader;
        binLoader.Export(path, model);

        // Write a .mat file for each extracted material
        std::filesystem::path fbxPath(path);
        for (const auto& mat : materials)
            WriteMaterialFile(fbxPath, mat);

        if (!materials.empty())
            LOG_INFO_CAT("Resource", "Extracted {} material(s) from {}", materials.size(), path);
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