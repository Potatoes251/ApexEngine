#include "MeshImporter.h"

#include "Log.h"

#include "Mesh.h"
#include "Model.h"

#include <fstream>
#include <filesystem>
#include <queue>

constexpr double ANIMATION_FPS = 30;

// ── Singleton FbxManager ──────────────────────────────────────
// FBX SDK allows exactly one FbxManager per process.
// Creating multiple managers (e.g. one per async import thread)
// causes undefined behavior and crashes.

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

bool MeshImporter::Import(std::string const& path, Model& model, std::vector<MaterialImportData>& materials)
{
    LOG_INFO_CAT("Resource", "Start importing {}", path);
    if (ImportFbx(path, model, materials))
    {
        LOG_INFO_CAT("Resource", "Successfully imported {}", path);
        return true;
    }
    // eventually load another way

    LOG_ERROR_CAT("Resource", "Fail to import \"{}\"", path);
    return false;
}

bool MeshImporter::ImportFbx(std::string const& path, Model& model, std::vector<MaterialImportData>& materials)
{
    // Serialize all FBX operations — the SDK is not thread-safe.
    std::lock_guard<std::mutex> lock(s_mutex);

    EnsureManager();

    FbxScene* scene = FbxScene::Create(s_manager, "");

    FbxImporter* importer = FbxImporter::Create(s_manager, "");

    if (!importer->Initialize(path.c_str(), -1, s_manager->GetIOSettings()))
    {
        model.SetState(ResourceState::Failed);
        importer->Destroy();
        scene->Destroy();
        return false;
    }

    if (!importer->Import(scene))
    {
        model.SetState(ResourceState::Failed);
        importer->Destroy();
        scene->Destroy();
        return false;
    }

    importer->Destroy();

    FbxGeometryConverter converter(s_manager);
    converter.Triangulate(scene, true);

    FbxAxisSystem::OpenGL.ConvertScene(scene);

    FbxNode* root = scene->GetRootNode();
    if (!root)
    {
        model.SetState(ResourceState::Failed);
        scene->Destroy();
        return false;
    }

    for (int i = 0; i < root->GetChildCount(); ++i)
    {
        ProcessNode(root->GetChild(i), model);
    }

    ResolveAndSortBones(scene, model);
    
    for (Mesh& mesh : model.m_meshes)
    {
        for (auto& vertex : mesh.m_vertices)
        {
            NormalizeVertexWeights(vertex);
        }
    }

    ExtractAnimations(scene, model);

    std::string fbxDir = std::filesystem::path(path).parent_path().generic_string();
    ExtractSceneMaterials(scene, fbxDir, materials);

    m_ctrlPointToVertices.clear();

    std::unordered_map<std::string, int> nameCounts;
    for (Mesh& mesh : model.m_meshes)
    {
        int& count = nameCounts[mesh.m_name];
        if (count > 0)
            mesh.m_name = mesh.m_name + " (" + std::to_string(count) + ")";
        count++;
    }

    scene->Destroy();

    model.SetState(ResourceState::Loaded);

    return true;
}

void MeshImporter::ProcessNode(FbxNode* node, Model& model)
{
    if (!node)
        return;

    FbxMesh* fbxMesh = node->GetMesh();

    if (fbxMesh)
    {
        m_ctrlPointToVertices.clear();

        model.m_meshes.emplace_back();
        Mesh& mesh = model.m_meshes.back();
        mesh.m_name = node->GetName();

        ProcessMesh(fbxMesh, mesh);
        ExtractBones(fbxMesh, model, mesh);
    }

    for (int i = 0; i < node->GetChildCount(); ++i)
    {
        ProcessNode(node->GetChild(i), model);
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

            vertex.m_position = { static_cast<float>(position[0]), static_cast<float>(position[1]), static_cast<float>(position[2]), 0 };

            FbxVector4 normal;
            fbxMesh->GetPolygonVertexNormal(i, j, normal);

            vertex.m_normal = { static_cast<float>(normal[0]), static_cast<float>(normal[1]), static_cast<float>(normal[2]), 0 };

            FbxStringList uvSetNames;
            fbxMesh->GetUVSetNames(uvSetNames);

            if (uvSetNames.GetCount() > 0)
            {
                FbxVector2 uv;
                bool unmapped;

                fbxMesh->GetPolygonVertexUV(i, j, uvSetNames[0], uv, unmapped);

                vertex.m_position[3] = static_cast<float>(uv[0]);
                vertex.m_normal[3] = static_cast<float>(uv[1]);
            }

            int vertexIndex = static_cast<int>(mesh.m_vertices.size());

            mesh.m_vertices.push_back(vertex);
            mesh.m_indices.push_back(vertexCounter++);

            m_ctrlPointToVertices[controlPointIndex].push_back(vertexIndex);
        }
    }
}

void MeshImporter::NormalizeVertexWeights(Apex::Rendering::Vertex& vertex)
{
    float sum = 0.0f;

    for (int i = 0; i < 4; i++)
        sum += vertex.m_weights[i];

    if (sum > 0.0f)
    {
        for (int i = 0; i < 4; i++)
            vertex.m_weights[i] /= sum;
    }
}

void MeshImporter::ResolveAndSortBones(FbxScene* scene, Model& model)
{
    // Build a full scene node map
    std::unordered_map<std::string, FbxNode*> nodeMap;
    BuildNodeMap(scene->GetRootNode(), nodeMap);

    const int boneCount = static_cast<int>(model.m_skeleton.size());

    // Second pass: resolve every bone's parent now that all are registered
    for (auto& bone : model.m_skeleton)
    {
        bone.m_parentIndex = -1;
        bone.m_children.clear();
    }

    for (int i = 0; i < boneCount; ++i)
    {
        auto nodeIt = nodeMap.find(m_boneNames[i]);
        if (nodeIt == nodeMap.end()) continue;

        FbxNode* parent = nodeIt->second->GetParent();
        if (!parent) continue;

        auto parentIt = model.m_boneMap.find(parent->GetName());
        if (parentIt == model.m_boneMap.end()) continue;

        int parentIdx = parentIt->second;
        model.m_skeleton[i].m_parentIndex = parentIdx;
        model.m_skeleton[parentIdx].m_children.push_back(i);
    }

    // Topological sort so every parent precedes its children
    std::vector<int> inDegree(boneCount, 0);
    for (int i = 0; i < boneCount; ++i)
        if (model.m_skeleton[i].m_parentIndex != -1)
            ++inDegree[i];

    std::queue<int> queue;
    for (int i = 0; i < boneCount; ++i)
        if (inDegree[i] == 0)
            queue.push(i);

    std::vector<int> sortedOrder;
    sortedOrder.reserve(boneCount);

    while (!queue.empty())
    {
        int cur = queue.front(); queue.pop();
        sortedOrder.push_back(cur);

        for (int child : model.m_skeleton[cur].m_children)
            if (--inDegree[child] == 0)
                queue.push(child);
    }

    // Build old new index remap
    std::vector<int> remap(boneCount);
    for (int newIdx = 0; newIdx < boneCount; ++newIdx)
        remap[sortedOrder[newIdx]] = newIdx;

    // Reorder bones and names arrays
    std::vector<BoneInfo>    sortedBones(boneCount);
    std::vector<std::string> sortedNames(boneCount);

    for (int oldIdx = 0; oldIdx < boneCount; ++oldIdx)
    {
        int newIdx = remap[oldIdx];
        sortedBones[newIdx] = model.m_skeleton[oldIdx];
        sortedNames[newIdx] = m_boneNames[oldIdx];
    }

    for (auto& bone : sortedBones)
    {
        if (bone.m_parentIndex != -1)
            bone.m_parentIndex = remap[bone.m_parentIndex];

        for (int& child : bone.m_children)
            child = remap[child];
    }

    model.m_skeleton = std::move(sortedBones);
    m_boneNames = std::move(sortedNames);

    // Rebuild boneMap to reflect new indices
    model.m_boneMap.clear();
    for (int i = 0; i < boneCount; ++i)
        model.m_boneMap[m_boneNames[i]] = i;

    for (Mesh& mesh : model.m_meshes)
    {
        // Remap bone IDs in every vertex
        for (auto& vertex : mesh.m_vertices)
            for (int i = 0; i < 4; ++i)
                if (vertex.m_weights[i] > 0.0f)
                    vertex.m_boneIDs[i] = remap[vertex.m_boneIDs[i]];
    }
}

void MeshImporter::ExtractBones(FbxMesh* fbxMesh, Model& model, Mesh& mesh)
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

            if (model.m_boneMap.find(boneName) == model.m_boneMap.end())
            {
                boneIndex = static_cast<int>(model.m_skeleton.size());

                model.m_boneMap[boneName] = boneIndex;
                m_boneNames.push_back(boneName);

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

                model.m_skeleton.push_back(boneInfo);
            }
            else
            {
                boneIndex = model.m_boneMap[boneName];
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

    int smallestIndex = 0;
    for (int i = 1; i < 4; i++)
    {
        if (vertex.m_weights[i] < vertex.m_weights[smallestIndex])
            smallestIndex = i;
    }

    if (weight > vertex.m_weights[smallestIndex])
    {
        vertex.m_boneIDs[smallestIndex] = boneID;
        vertex.m_weights[smallestIndex] = weight;
    }
}

void MeshImporter::BuildNodeMap(FbxNode* node, std::unordered_map<std::string, FbxNode*>& map)
{
    if (!node) return;

    map[node->GetName()] = node;

    for (int i = 0; i < node->GetChildCount(); ++i)
    {
        BuildNodeMap(node->GetChild(i), map);
    }
}

void MeshImporter::ExtractAnimations(FbxScene* scene, Model& model)
{
    constexpr double FPS = ANIMATION_FPS;

    int animStackCount = scene->GetSrcObjectCount<FbxAnimStack>();
    if (animStackCount == 0) return;

    FbxNode* root = scene->GetRootNode();
    if (!root) return;

    std::unordered_map<std::string, FbxNode*> nodeMap;
    BuildNodeMap(root, nodeMap);

    model.m_animations.clear();
    model.m_animations.resize(animStackCount);

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

        Animation& anim = model.m_animations[animIndex];
        anim.m_name = stack->GetName();

        const int boneCount = static_cast<int>(model.m_skeleton.size());
        anim.m_boneAnimations.resize(boneCount);

        for (int bone = 0; bone < boneCount; ++bone)
        {
            const std::string& boneName = m_boneNames[bone];

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
                    key.m_rotation = LibMath::Quaternion::identity();
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
            }
        }
    }
}

FbxAMatrix MeshImporter::GetGeometryTransform(FbxNode* node)
{
    FbxVector4 t = node->GetGeometricTranslation(FbxNode::eSourcePivot);
    FbxVector4 r = node->GetGeometricRotation(FbxNode::eSourcePivot);
    FbxVector4 s = node->GetGeometricScaling(FbxNode::eSourcePivot);

    return FbxAMatrix(t, r, s);
}

void MeshImporter::ExtractSceneMaterials(FbxScene* scene, const std::string& fbxDir, std::vector<MaterialImportData>& materials)
{
    int matCount = scene->GetMaterialCount();
    for (int i = 0; i < matCount; ++i)
    {
        FbxSurfaceMaterial* fbxMat = scene->GetMaterial(i);
        if (!fbxMat) continue;

        MaterialImportData data;
        data.m_name = fbxMat->GetName();

        data.m_diffusePath = ResolveTexturePath(fbxMat, FbxSurfaceMaterial::sDiffuse, fbxDir);
        data.m_normalPath = ResolveTexturePath(fbxMat, FbxSurfaceMaterial::sNormalMap, fbxDir);
        data.m_specularPath = ResolveTexturePath(fbxMat, FbxSurfaceMaterial::sSpecular, fbxDir);

        // Some exporters use "sSpecularFactor" or "sShininess" — try sDiffuseFactor as fallback
        if (data.m_diffusePath.empty())
            data.m_diffusePath = ResolveTexturePath(fbxMat, FbxSurfaceMaterial::sDiffuseFactor, fbxDir);

        if (data.m_diffusePath.empty())
        {
            if (fbxMat->GetClassId().Is(FbxSurfaceLambert::ClassId))
            {
                FbxDouble3 c = static_cast<FbxSurfaceLambert*>(fbxMat)->Diffuse.Get();
                data.m_tint = { (float)c[0], (float)c[1], (float)c[2] };
            }
        }

        if (!data.m_diffusePath.empty() || !data.m_normalPath.empty() || !data.m_specularPath.empty() && data.m_tint == LibMath::Vector3{ 1, 1, 1 })
            materials.push_back(std::move(data));
    }
}

std::string MeshImporter::ResolveTexturePath(FbxSurfaceMaterial* mat, const char* propName, const std::string& fbxDir)
{
    FbxProperty prop = mat->FindProperty(propName);
    if (!prop.IsValid()) return {};

    int texCount = prop.GetSrcObjectCount<FbxFileTexture>();
    if (texCount == 0) return {};

    FbxFileTexture* tex = prop.GetSrcObject<FbxFileTexture>(0);
    if (!tex) return {};

    // Prefer relative filename so paths survive moving the project
    std::string relPath = tex->GetRelativeFileName();
    std::string absPath = tex->GetFileName();

    // Normalize path separators
    for (auto& c : relPath) 
        if (c == '\\') 
            c = '/';
    for (auto& c : absPath) 
        if (c == '\\') 
            c = '/';

    // If the relative path exists alongside the FBX, use it
    if (!relPath.empty() && relPath != "." && relPath != "./")
    {
        std::filesystem::path candidate =
            std::filesystem::path(fbxDir) / std::filesystem::path(relPath);
        if (std::filesystem::exists(candidate))
            return candidate.generic_string();
    }

    // Fall back to absolute path
    if (!absPath.empty())
    {
        if (std::filesystem::exists(absPath))
            return absPath;
    }

    return {};
}

LibMath::Matrix4 MeshImporter::ConvertFBXMatrix(const FbxAMatrix& mat)
{
    LibMath::Matrix4 result;

    for (int row = 0; row < 4; row++)
    {
        for (int col = 0; col < 4; col++)
        {
            result[row][col] = static_cast<float>(mat.Get(row, col));
        }
    }

    return result;
}
