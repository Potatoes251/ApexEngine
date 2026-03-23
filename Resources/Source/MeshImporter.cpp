#include "MeshImporter.h"

#include "Vertex.h"

#include <fstream>
#include <filesystem>

// ── Singleton FbxManager ──────────────────────────────────────
// FBX SDK allows exactly one FbxManager per process.
// Creating multiple managers (e.g. one per async import thread)
// causes undefined behaviour and crashes.
FbxManager* MeshImporter::s_manager = nullptr;
std::mutex  MeshImporter::s_mutex;

static void EnsureManager()
{
    // Called inside the locked region — safe to check/create once.
    if (!MeshImporter::s_manager)
    {
        MeshImporter::s_manager = FbxManager::Create();
        FbxIOSettings* ios = FbxIOSettings::Create(MeshImporter::s_manager, IOSROOT);
        MeshImporter::s_manager->SetIOSettings(ios);
    }
}

void MeshImporter::Import(std::string path, Mesh& mesh)
{
    if (ImportBin(path, mesh)) return;
    if (ImportFbx(path, mesh))
    {
        ExportBin(path, mesh);
        return;
    }
    // eventually load another way
}

bool MeshImporter::ImportBin(std::string path, Mesh& mesh)
{
    if (IsBigEndian()) return false;
    std::filesystem::path binaryPath = path;
    binaryPath.replace_extension(".mesh");
    std::ifstream file(binaryPath, std::ios::binary);

    if (!file.is_open())
    {
        std::cout << "Failed to open file\n";
        return false;
    }

    file.seekg(0, std::ios::end);
    std::streamsize size = file.tellg();

    if (size == 0)
    {
        std::cout << "File is empty\n";
        return false;
    }

    file.seekg(0, std::ios::beg);

    MeshHeader header;
    file.read((char*)&header, sizeof(header));

    if (header.m_magic != 0x4D455348)
    {
        std::cout << "Invalid file format\n";
        return false;
    }
    if (header.m_version != CURRENT_VERSION)
    {
        std::cout << "File needs to be updated\n";
        return false;
    }
    uint32_t vertexCount;
    uint32_t indexCount;

    if (!file.read((char*)&vertexCount, sizeof(vertexCount)))
        return false;

    if (!file.read((char*)&indexCount, sizeof(indexCount)))
        return false;

    std::vector<Apex::Rendering::Vertex> vertices(vertexCount);

    if (!file.read((char*)vertices.data(), vertexCount * sizeof(Apex::Rendering::Vertex)))
        return false;

    std::vector<uint32_t> indices(indexCount);

    if (!file.read((char*)indices.data(), indexCount * sizeof(uint32_t)))
        return false;

    mesh.m_vertices = vertices;
    mesh.m_indices = indices;

    return true;
}

bool MeshImporter::ExportBin(std::string path, Mesh& mesh)
{
    std::filesystem::path binaryPath = path;
    binaryPath.replace_extension(".mesh");

    std::ofstream file(binaryPath, std::ios::binary);

    if (!file.is_open())
    {
        std::cout << "Failed to open file for writing\n";
        return false;
    }

    // Header
    MeshHeader header;
    header.m_magic = 0x4D455348; // "MESH"
    header.m_version = CURRENT_VERSION;

    file.write((char*)&header, sizeof(header));

    // Counts
    uint32_t vertexCount = mesh.m_vertices.size();
    uint32_t indexCount = mesh.m_indices.size();

    file.write((char*)&vertexCount, sizeof(vertexCount));
    file.write((char*)&indexCount, sizeof(indexCount));

    // Data
    file.write((char*)mesh.m_vertices.data(), vertexCount * sizeof(Apex::Rendering::Vertex));
    file.write((char*)mesh.m_indices.data(), indexCount * sizeof(uint32_t));

    return true;
}

bool MeshImporter::ImportFbx(std::string path, Mesh& mesh)
{
    // Serialize all FBX operations — the SDK is not thread-safe.
    std::lock_guard<std::mutex> lock(s_mutex);

    EnsureManager();

    FbxScene* scene = FbxScene::Create(s_manager, "");

    FbxImporter* importer = FbxImporter::Create(s_manager, "");

    if (!importer->Initialize(path.c_str(), -1, s_manager->GetIOSettings()))
    {
        mesh.SetState(ResourceState::Failed);
        importer->Destroy();
        scene->Destroy();
        return false;
    }

    if (!importer->Import(scene))
    {
        mesh.SetState(ResourceState::Failed);
        importer->Destroy();
        scene->Destroy();
        return false;
    }

    importer->Destroy();

    FbxGeometryConverter converter(s_manager);
    converter.Triangulate(scene, true);

    FbxAxisSystem::OpenGL.ConvertScene(scene);
    FbxSystemUnit::m.ConvertScene(scene);

    FbxNode* root = scene->GetRootNode();
    if (!root)
    {
        mesh.SetState(ResourceState::Failed);
        scene->Destroy();
        return false;
    }

    for (int i = 0; i < root->GetChildCount(); ++i)
    {
        ProcessNode(root->GetChild(i), mesh);
    }

    scene->Destroy();

    return true;
}

void MeshImporter::ProcessNode(FbxNode* node, Mesh& mesh)
{
    if (!node)
        return;

    FbxMesh* fbxMesh = node->GetMesh();

    if (fbxMesh)
        ProcessMesh(fbxMesh, mesh);

    for (int i = 0; i < node->GetChildCount(); ++i)
    {
        ProcessNode(node->GetChild(i), mesh);
    }
}

void MeshImporter::ProcessMesh(FbxMesh* fbxMesh, Mesh& mesh)
{
    FbxVector4* controlPoints = fbxMesh->GetControlPoints();
    int polygonCount = fbxMesh->GetPolygonCount();

    int vertexCounter = static_cast<int>(mesh.m_vertices.size());

    for (int i = 0; i < polygonCount; ++i)
    {
        int polygonSize = fbxMesh->GetPolygonSize(i);

        for (int j = 0; j < polygonSize; ++j)
        {
            int controlPointIndex = fbxMesh->GetPolygonVertex(i, j);
            FbxVector4 position = controlPoints[controlPointIndex];

            Apex::Rendering::Vertex vertex{};

            vertex.m_position = { static_cast<float>(position[0]), static_cast<float>(position[1]),static_cast<float>(position[2]) };

            FbxVector4 normal;
            fbxMesh->GetPolygonVertexNormal(i, j, normal);

            vertex.m_normal = { static_cast<float>(normal[0]), static_cast<float>(normal[1]), static_cast<float>(normal[2]) };

            FbxStringList uvSetNames;
            fbxMesh->GetUVSetNames(uvSetNames);

            if (uvSetNames.GetCount() > 0)
            {
                FbxVector2 uv;
                bool unmapped;

                fbxMesh->GetPolygonVertexUV(i, j, uvSetNames[0], uv, unmapped);

                vertex.m_uv = { static_cast<float>(uv[0]), static_cast<float>(uv[1]) };
            }

            mesh.m_vertices.push_back(vertex);
            mesh.m_indices.push_back(vertexCounter++);
        }
    }
}

bool IsBigEndian()
{
    union
    {
        int in;
        char ch[4];
    } bint = { 0x01020304 };

    return bint.ch[0] == 1;
}
