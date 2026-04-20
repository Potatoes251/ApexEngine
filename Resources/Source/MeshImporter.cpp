#include "MeshImporter.h"

#include "Vertex.h"

#include "Log.h"

#include <fstream>
#include <filesystem>

constexpr double ANIMATION_FPS = 30;

// ── Singleton FbxManager ──────────────────────────────────────
// FBX SDK allows exactly one FbxManager per process.
// Creating multiple managers (e.g. one per async import thread)
// causes undefined behaviour and crashes.

void MeshImporter::EnsureManager()
{
    // Called inside the locked region — safe to check/create once.
    if (!s_manager)
    {
        s_manager = FbxManager::Create();
        FbxIOSettings* ios = FbxIOSettings::Create(s_manager, IOSROOT);
        s_manager->SetIOSettings(ios);
    }
}

bool MeshImporter::Import(std::string path, Mesh& mesh)
{
    LOG_INFO_CAT("Resource", "Start importing {}", path);
    if (ImportFbx(path, mesh))
    {
        return true;
    }
    // eventually load another way


    LOG_ERROR_CAT("Resource", "Fail to import {}", path);
    return false;
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
        LOG_ERROR_CAT("Resource", "failed to import {}", path);
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

    ExtractAnimations(scene, mesh);

    m_ctrlPointToVertices.clear();

    scene->Destroy();

    return true;
}

void MeshImporter::ProcessNode(FbxNode* node, Mesh& mesh)
{
    if (!node)
        return;

    FbxMesh* fbxMesh = node->GetMesh();

    if (fbxMesh)
    {
        ProcessMesh(fbxMesh, mesh);
        ExtractBones(fbxMesh, mesh);
    }

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

            int vertexIndex = static_cast<int>(mesh.m_vertices.size());

            mesh.m_vertices.push_back(vertex);
            mesh.m_indices.push_back(vertexCounter++);

            m_ctrlPointToVertices[controlPointIndex].push_back(vertexIndex);
        }
    }
}

void MeshImporter::ExtractBones(FbxMesh* fbxMesh, Mesh& mesh)
{
    int skinCount = fbxMesh->GetDeformerCount(FbxDeformer::eSkin);

    for (int i = 0; i < skinCount; i++)
    {
        FbxSkin* skin = static_cast<FbxSkin*>(fbxMesh->GetDeformer(i, FbxDeformer::eSkin));

        int clusterCount = skin->GetClusterCount();

        for (int j = 0; j < clusterCount; j++)
        {
            FbxCluster* cluster = skin->GetCluster(j);
            FbxNode*    boneNode = cluster->GetLink();

            if (!boneNode) continue;

            std::string boneName = boneNode->GetName();

            int boneIndex = 0;

            if (mesh.m_boneMap.find(boneName) == mesh.m_boneMap.end())
            {
                boneIndex = static_cast<int>(mesh.m_bones.size());

                mesh.m_boneMap[boneName] = boneIndex;
                mesh.m_boneNames.push_back(boneName);

                BoneInfo boneInfo;

                //offset matrix
                boneInfo.m_parentIndex = -1;

                FbxAMatrix meshTransform;
                FbxAMatrix boneTransform;

                cluster->GetTransformMatrix(meshTransform);          // mesh global at bind pose
                cluster->GetTransformLinkMatrix(boneTransform);      // bone global at bind pose

                FbxAMatrix geom;
                FbxNode* meshNode = fbxMesh->GetNode();
                geom.SetT(meshNode->GetGeometricTranslation(FbxNode::eSourcePivot));
                geom.SetR(meshNode->GetGeometricRotation(FbxNode::eSourcePivot));
                geom.SetS(meshNode->GetGeometricScaling(FbxNode::eSourcePivot));

                FbxAMatrix m_offsetMatrix = boneTransform.Inverse() * meshTransform * geom;

                boneInfo.m_offsetMatrix = ConvertFBXMatrix(m_offsetMatrix);

                if (boneNode->GetParent()) //check if node has a parent
                {
                    std::string parentName = boneNode->GetParent()->GetName();

                    auto it = mesh.m_boneMap.find(parentName);
                    if (it != mesh.m_boneMap.end())
                    {
                        //set the parent index
                        boneInfo.m_parentIndex = it->second;

                        //add the present index to the parent
                        int currentIndex = static_cast<int>(mesh.m_bones.size());
                        mesh.m_bones[it->second].m_children.push_back(currentIndex);
                    }
                }
                mesh.m_bones.push_back(boneInfo);
            }
            else
            {
                boneIndex = mesh.m_boneMap[boneName];
            }

            int*    controlPointIndices = cluster->GetControlPointIndices();
            double* weights             = cluster->GetControlPointWeights();
            int     indexCount          = cluster->GetControlPointIndicesCount();

            for (int k = 0; k < indexCount; k++)
            {
                int     ctrlIndex = controlPointIndices[k];
                double  weight = weights[k];

                auto it = m_ctrlPointToVertices.find(ctrlIndex);

                if (it != m_ctrlPointToVertices.end())
                {
                    for (int vertexID : it->second)
                    {
                        AddBoneWeight(mesh, vertexID, boneIndex, static_cast<float>(weight));
                    }
                }
            }
        }
    }
}

void MeshImporter::AddBoneWeight(Mesh& mesh, int vertexID, int boneID, float weight)
{
    Apex::Rendering::Vertex& vertex = mesh.m_vertices[vertexID];

    for (int i = 0; i < 4; i++)
    {
        if (vertex.m_weights[i] == 0.0f)
        {
            vertex.m_boneIDs[i] = boneID;
            vertex.m_weights[i] = weight;
            return;
        }
    }
}

void BuildNodeMap(FbxNode* node, std::unordered_map<std::string, FbxNode*>& map)
{
    if (!node) return;

    map[node->GetName()] = node;

    for (int i = 0; i < node->GetChildCount(); ++i)
    {
        BuildNodeMap(node->GetChild(i), map);
    }
}

void MeshImporter::ExtractAnimations(FbxScene* scene, Mesh& mesh)
{
    constexpr double FPS = ANIMATION_FPS;

    int animStackCount = scene->GetSrcObjectCount<FbxAnimStack>();
    if (animStackCount == 0) return;

    FbxNode* root = scene->GetRootNode();
    if (!root) return;

    std::unordered_map<std::string, FbxNode*> nodeMap;
    BuildNodeMap(root, nodeMap);

    mesh.m_animations.clear();
    mesh.m_animations.resize(animStackCount);

    for (int animIndex = 0; animIndex < animStackCount; ++animIndex)
    {
        FbxAnimStack* stack = scene->GetSrcObject<FbxAnimStack>(animIndex);
        scene->SetCurrentAnimationStack(stack);

        FbxTakeInfo* takeInfo = scene->GetTakeInfo(stack->GetName());
        if (!takeInfo) continue;

        const double startSec = takeInfo->mLocalTimeSpan.GetStart().GetSecondDouble();
        const double endSec = takeInfo->mLocalTimeSpan.GetStop().GetSecondDouble();

        const double duration = endSec - startSec;
        const int frameCount = static_cast<int>(duration * FPS);

        if (frameCount <= 0) continue;

        Animation& anim = mesh.m_animations[animIndex];
        anim.m_name = stack->GetName();

        const int boneCount = static_cast<int>(mesh.m_bones.size());
        anim.m_boneAnimations.resize(boneCount);

        for (int bone = 0; bone < boneCount; ++bone)
        {
            const std::string& boneName = mesh.m_boneNames[bone];

            auto& boneAnim = anim.m_boneAnimations[bone];
            boneAnim.m_frames.resize(frameCount);

            FbxNode* node = nullptr;
            auto it = nodeMap.find(boneName);
            if (it != nodeMap.end())
                node = it->second;

            if (!node)
            {
                // Fill identity once
                for (auto& key : boneAnim.m_frames)
                {
                    key.m_position = { 0,0,0 };
                    key.m_rotation = Quaternion::identity();
                    key.m_scale = { 1,1,1 };
                }
                continue;
            }

            FbxTime time;

            for (int frame = 0; frame < frameCount; ++frame)
            {
                time.SetSecondDouble(startSec + frame / FPS);

                FbxAMatrix local = node->EvaluateLocalTransform(time);

                FbxVector4 t = local.GetT();
                FbxQuaternion r = local.GetQ();
                FbxVector4 s = local.GetS();

                KeyFrame& key = boneAnim.m_frames[frame];

                key.m_position = { (float)t[0], (float)t[1], (float)t[2] };
                key.m_rotation = { (float)r[0], (float)r[1], (float)r[2], (float)r[3] };
                key.m_scale = { (float)s[0], (float)s[1], (float)s[2] };

                // ❌ REMOVED:
                // EvaluateGlobalTransform
                // matrix conversion (expensive)
            }
        }
    }
}

const std::string& MeshImporter::GetBoneNameFromIndex(const Mesh& mesh, int index)
{
    for (const auto& [name, i] : mesh.m_boneMap)
    {
        if (i == index) return name;
    }

    static std::string empty = "";
    return empty;
}

FbxAMatrix MeshImporter::GetGeometryTransform(FbxNode* node)
{
    FbxVector4 t = node->GetGeometricTranslation(FbxNode::eSourcePivot);
    FbxVector4 r = node->GetGeometricRotation(FbxNode::eSourcePivot);
    FbxVector4 s = node->GetGeometricScaling(FbxNode::eSourcePivot);

    return FbxAMatrix(t, r, s);
}

Matrix4 MeshImporter::ConvertFBXMatrix(const FbxAMatrix& mat)
{
    Matrix4 result;

    for (int row = 0; row < 4; row++)
    {
        for (int col = 0; col < 4; col++)
        {
            result[row][col] = static_cast<float>(mat.Get(row, col));
        }
    }

    return result;
}
