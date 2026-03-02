#include "MeshImporter.h"

#include <mutex>

void MeshImporter::Import(std::string path, Mesh& mesh)
{
    FbxManager* manager = nullptr;
    FbxIOSettings* ios = nullptr;
    
    static std::mutex mutex;
    std::lock_guard<std::mutex> lock(mutex);

    manager = FbxManager::Create();
    ios = FbxIOSettings::Create(manager, IOSROOT);
	manager->SetIOSettings(ios);


	FbxScene* scene = FbxScene::Create(manager, "");

	FbxImporter* importer = FbxImporter::Create(manager, "");

    if (!importer->Initialize(path.c_str(), -1, manager->GetIOSettings())) 
    {
        mesh.SetState(ResourceState::Failed);
        importer->Destroy();
        manager->Destroy();
        return;
    }

    if (!importer->Import(scene)) 
    {
        mesh.SetState(ResourceState::Failed);
        importer->Destroy();
        manager->Destroy();
        return;
    }

	importer->Destroy();

    FbxGeometryConverter converter(manager);
    converter.Triangulate(scene, true);

    FbxAxisSystem::OpenGL.ConvertScene(scene);
    FbxSystemUnit::m.ConvertScene(scene);

    FbxNode* root = scene->GetRootNode();
    if (!root)
    {
        mesh.SetState(ResourceState::Failed);
        scene->Destroy();
        return;
    }

    for (int i = 0; i < root->GetChildCount(); ++i)
    {
        ProcessNode(root->GetChild(i), mesh);
    }

    scene->Destroy();
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

            Vertex vertex{};

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

                vertex.m_textureUV = { static_cast<float>(uv[0]), static_cast<float>(uv[1]) };
            }

            mesh.m_vertices.push_back(vertex);
            mesh.m_indices.push_back(vertexCounter++);
        }
    }
}