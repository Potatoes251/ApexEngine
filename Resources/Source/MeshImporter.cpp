#include "MeshImporter.h"

// includes Vertex
#include "RHI.h"

const double ANIMATION_FPS = 30;

// ── Singleton FbxManager ──────────────────────────────────────
// FBX SDK allows exactly one FbxManager per process.
// Creating multiple managers (e.g. one per async import thread)
// causes undefined behaviour and crashes.
// Initialized on first use via std::call_once; kept alive for
// the process lifetime as the SDK recommends.
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
    // Serialize all FBX operations — the SDK is not thread-safe.
    std::lock_guard<std::mutex> lock(s_mutex);

    EnsureManager();

	FbxScene* scene = FbxScene::Create(s_manager, "");

	FbxImporter* importer = FbxImporter::Create(s_manager, "");

    if (!importer->Initialize(path.c_str(), -1, s_manager->GetIOSettings()))
    {
        mesh.SetState(ResourceState::Failed);
        importer->Destroy();
        s_manager->Destroy();
        return;
    }

    if (!importer->Import(scene)) 
    {
        mesh.SetState(ResourceState::Failed);
        importer->Destroy();
        s_manager->Destroy();
        return;
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
        return;
    }

    for (int i = 0; i < root->GetChildCount(); ++i)
    {
        ProcessNode(root->GetChild(i), mesh);
    }

    ExtractAnimations(scene, mesh);

    m_ctrlPointToVertices.clear();

    scene->Destroy();
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

                mesh.m_boneMap[boneName] = boneIndex;;

                BoneInfo boneInfo;
                boneInfo.parentIndex = -1;

                FbxAMatrix meshTransform;
                FbxAMatrix boneTransform;

                cluster->GetTransformMatrix(meshTransform);          // mesh global at bind pose
                cluster->GetTransformLinkMatrix(boneTransform);      // bone global at bind pose

                FbxAMatrix offsetMatrix = boneTransform.Inverse() * meshTransform;

                boneInfo.offsetMatrix = ConvertFBXMatrix(offsetMatrix);

                if (boneNode->GetParent()) //check if node has a parent
                {
                    std::string parentName = boneNode->GetParent()->GetName();

                    auto it = mesh.m_boneMap.find(parentName);
                    if (it != mesh.m_boneMap.end())
                    {
                        boneInfo.parentIndex = it->second;
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

void MeshImporter::ExtractAnimations(FbxScene* scene, Mesh& mesh)
{
    int animStackCount = scene->GetSrcObjectCount<FbxAnimStack>();
    if (animStackCount == 0) return;

    FbxNode* root = scene->GetRootNode();
    if (!root) return;

    mesh.m_animations.clear();
    mesh.m_animations.resize(animStackCount);

    for (int animIndex = 0; animIndex < animStackCount; ++animIndex)
    {
        FbxAnimStack* stack = scene->GetSrcObject<FbxAnimStack>(animIndex);
        scene->SetCurrentAnimationStack(stack);

        FbxTakeInfo* takeInfo = scene->GetTakeInfo(stack->GetName());
        if (!takeInfo) continue;

        FbxTime start = takeInfo->mLocalTimeSpan.GetStart();
        FbxTime end = takeInfo->mLocalTimeSpan.GetStop();

        double duration = end.GetSecondDouble() - start.GetSecondDouble();
        int frameCount = static_cast<int>(duration * ANIMATION_FPS);

        if (frameCount <= 0) continue;

        Animation& anim = mesh.m_animations[animIndex];
        anim.m_name = stack->GetName();

        int boneCount = static_cast<int>(mesh.m_bones.size());
        anim.m_boneAnimations.resize(boneCount);

        for (int bone = 0; bone < boneCount; ++bone)
        {
            const std::string& boneName = GetBoneNameFromIndex(mesh, bone);

            auto& boneAnim = anim.m_boneAnimations[bone];
            boneAnim.m_frames.resize(frameCount);

            FbxNode* node = root->FindChild(boneName.c_str(), true);
            if (!node)
            {
                for (int frame = 0; frame < frameCount; ++frame)
                {
                    KeyFrame& key = boneAnim.m_frames[frame];
                    key.m_position = Vector3(0.f, 0.f, 0.f);
                    key.m_rotation = Quaternion::identity();
                    key.m_scale = Vector3(1.f, 1.f, 1.f);
                }
                continue;
            }

            for (int frame = 0; frame < frameCount; ++frame)
            {
                FbxTime time;
                time.SetSecondDouble(start.GetSecondDouble() + frame / ANIMATION_FPS);

                FbxAMatrix transform = node->EvaluateGlobalTransform(time);

                FbxVector4 t = transform.GetT();
                FbxQuaternion r = transform.GetQ();
                FbxVector4 s = transform.GetS();

                KeyFrame& key = boneAnim.m_frames[frame];

                key.m_position = Vector3((float)t[0], (float)t[1], (float)t[2]);

                key.m_rotation = Quaternion(
                    (float)r[3],
                    (float)r[0],
                    (float)r[1],
                    (float)r[2]);

                key.m_scale = Vector3((float)s[0], (float)s[1], (float)s[2]);

                key.m_matrix = ConvertFBXMatrix(transform);
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
