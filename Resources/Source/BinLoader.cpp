#include "BinLoader.h"

#include "Log.h"

#include <fstream>
#include <filesystem>

bool BinLoader::Load(std::string const& path, Model& model)
{
    std::ifstream file(path, std::ios::binary);

    if (!file.is_open())
    {
        LOG_ERROR_CAT("Resource", "Failed to open bin file : {}", path);
        return false;
    }

    file.seekg(0, std::ios::end);
    std::streamsize size = file.tellg();

    if (size == 0)
    {
        LOG_ERROR_CAT("Resource", "File is empty : {}", path);
        return false;
    }

    file.seekg(0, std::ios::beg);

    uint32_t version;
    if (!ReadHeader(file, version)) return false;

    if (version == LEGACY_VERSION)
    {
        if (!LoadLegacy(file, model)) return false;
        if (!Export(path, model)) return false;
        LoadAnimations(path, model);  // loading animations after avoid reexporting the animations
        return true;
    }

    ReadSkeleton(model, file);
    ReadBoneMap(model, file);

    uint32_t meshCount;

    Read32Bits(file, meshCount);

    for (int32_t i = 0; i < meshCount; i++)
    {
        model.m_meshes.emplace_back();
        Mesh& mesh = model.m_meshes.back();

        if (!ReadString(file, mesh.m_name)) return false;
        if (!ReadVerticesIndices(mesh, file)) return false;
    }

    LoadAnimations(path, model);

    return true;
}

bool BinLoader::LoadLegacy(std::ifstream& file, Model& model)
{
    model.m_meshes.emplace_back();
    Mesh& mesh = model.m_meshes.back();
    mesh.m_name = "default";

    if (!ReadVerticesIndices(mesh, file)
        || !ReadSkeleton(model, file)
        || !ReadBoneMap(model, file))
        return false;

    return true;
}

bool BinLoader::Export(std::string const& path, Model const& model)
{
    LOG_INFO_CAT("Resource", "Start exporting {} to bin", path.c_str());
    std::filesystem::path binaryPath = path;
    binaryPath.replace_extension(".mesh");
    std::filesystem::path folderPath = binaryPath.parent_path();

    if (folderPath.filename() != binaryPath.stem())
    {
        folderPath /= binaryPath.stem();
        if (!std::filesystem::exists(folderPath))
        {
            std::filesystem::create_directory(folderPath);
        }
    }

    std::ofstream file(folderPath / binaryPath.filename(), std::ios::binary);

    if (!file.is_open())
    {
        LOG_ERROR_CAT("Resource", "Failed to open file '{}' for writing", path);
        return false;
    }

    WriteHeader(file);
    WriteSkeleton(model, file);
    WriteBoneMap(model, file);

    uint32_t meshCount = model.m_meshes.size();
    Write32Bits(file, meshCount);

    for (Mesh const& mesh : model.m_meshes)
    {
        WriteString(file, mesh.m_name);
        WriteVerticesIndices(mesh, file);
    }

    if (model.m_animations.size() == 0) return true;

    folderPath /= "Animation";
    if (!std::filesystem::exists(folderPath)) 
    {
        std::filesystem::create_directory(folderPath);
    }

    WriteAnimations(model, folderPath);

    return true;
}


bool BinLoader::ReadHeader(std::ifstream& file, uint32_t& version)
{
    MeshHeader header;

    if (!Read32Bits(file, header.m_magic)) return false;
    if (!Read32Bits(file, header.m_version)) return false;
    if (!Read32Bits(file, header.m_vertexSize)) return false;
    if (!Read32Bits(file, header.m_keyframeSize)) return false;
    
    if (header.m_version < 7) // remove at version 7
    {
        std::string sourcePath;
        if (!ReadString(file, sourcePath)) return false;
    }

    version = header.m_version;

    if (header.m_magic != 0x4D455348)
    {
        LOG_ERROR_CAT("Resource", "Invalid file format");
        return false;
    }
    if (header.m_version != CURRENT_VERSION && header.m_version != LEGACY_VERSION)
    {
        LOG_ERROR_CAT("Resource", "File needs to be updated (version was {})", header.m_version);
        return false;
    }
    if (header.m_vertexSize != sizeof(Apex::Rendering::Vertex)
        || header.m_keyframeSize != sizeof(KeyFrame))
    {
        LOG_ERROR_CAT("Resource", "Struct size doesn't match");
        return false;
    }

    return true;
}

bool BinLoader::ReadVerticesIndices(Mesh& mesh, std::ifstream& file)
{
    uint32_t vertexCount;
    uint32_t indexCount;

    if (!Read32Bits(file, vertexCount))
        return false;

    if (!Read32Bits(file, indexCount))
        return false;

    std::vector<Apex::Rendering::Vertex> vertices(vertexCount);

    if (!file.read((char*)vertices.data(), vertexCount * sizeof(Apex::Rendering::Vertex)))
        return false;

    std::vector<uint32_t> indices(indexCount);

    if (!file.read((char*)indices.data(), indexCount * sizeof(uint32_t)))
        return false;

    // swap all byte if big endian
    if (IsBigEndian())
    {
        uint32_t* data = reinterpret_cast<uint32_t*>(indices.data());

        for (uint32_t i = 0; i < indexCount; i++)
        {
            data[i] = _byteswap_ulong(data[i]);
        }

        static_assert(alignof(Apex::Rendering::Vertex) == 4);
        data = reinterpret_cast<uint32_t*>(vertices.data());
        size_t totalU32 = (vertices.size() * sizeof(Apex::Rendering::Vertex)) / 4;

        for (size_t i = 0; i < totalU32; i++)
        {
            data[i] = _byteswap_ulong(data[i]);
        }
    }

    mesh.m_vertices = vertices;
    mesh.m_indices = indices;

    return true;
}

bool BinLoader::ReadSkeleton(Model& model, std::ifstream& file)
{
    uint32_t boneCount;

    if (!Read32Bits(file, boneCount)) return false;

    std::vector<BoneInfo> bones(boneCount);

    for (uint32_t i = 0; i < boneCount; i++)
    {
        if (!file.read((char*)&bones[i].m_offsetMatrix, sizeof(LibMath::Matrix4)))
            return false;
        
        // swap all byte if big endian
        if (IsBigEndian())
        {
            uint32_t* data = reinterpret_cast<uint32_t*>(&bones[i].m_offsetMatrix[0][0]);

            constexpr size_t totalU32 = sizeof(LibMath::Matrix4) / 4;

            for (size_t j = 0; j < totalU32; j++)
            {
                data[j] = _byteswap_ulong(data[j]);
            }
        }

        if (!Read32Bits(file, *reinterpret_cast<uint32_t*>(&bones[i].m_parentIndex)))
            return false;

        uint32_t nbChild;
        if (!Read32Bits(file, nbChild))
            return false;

        bones[i].m_children.resize(nbChild);
        if (!file.read((char*)bones[i].m_children.data(), nbChild * sizeof(uint32_t)))
            return false;

        // swap all byte if big endian
        if (IsBigEndian())
        {
            uint32_t* data = reinterpret_cast<uint32_t*>(bones[i].m_children.data());

            for (size_t j = 0; j < bones[i].m_children.size(); j++)
            {
                data[j] = _byteswap_ulong(data[j]);
            }
        }
    }

    model.m_skeleton = bones;

    return true;
}

bool BinLoader::ReadBoneMap(Model& model, std::ifstream& file)
{
    uint32_t count;

    if (!Read32Bits(file, count)) return false;

    model.m_boneMap.reserve(count);

    for (uint32_t i = 0; i < count; i++)
    {
        std::string key;
        uint32_t value;

        if (!ReadString(file, key)) return false;
        if (!Read32Bits(file, value)) return false;

        model.m_boneMap.emplace(std::move(key), value);
    }

    return true;
}

bool BinLoader::ReadAnimation(Model& model, std::filesystem::path const& path)
{
    std::ifstream file(path, std::ios::binary);

    if (!file.is_open())
    {
        LOG_ERROR_CAT("Resource", "Failed to open file '{}' for reading", path.string());
        return false;
    }

    Animation animation;

    animation.m_name = path.stem().string();

    uint32_t count;

    if (!Read32Bits(file, count)) return false;

    animation.m_boneAnimations.resize(count);

    for (size_t j = 0; j < count; j++)
    {
        if (!ReadSingleBoneAnimation(animation.m_boneAnimations[j], file)) return false;
    }

    model.m_animations.push_back(animation);

    return true;
}

bool BinLoader::ReadSingleBoneAnimation(SingleBoneAnimation& sba, std::ifstream& file)
{
    uint32_t count;

    if (!Read32Bits(file, count)) return false;

    sba.m_frames.resize(count);

    if (!file.read((char*)sba.m_frames.data(), count * sizeof(KeyFrame))) return false;

    if (IsBigEndian())
    {
        uint32_t* data = reinterpret_cast<uint32_t*>(sba.m_frames.data());
        size_t totalU32 = (sba.m_frames.size() * sizeof(KeyFrame)) / 4;

        for (size_t i = 0; i < totalU32; i++)
        {
            data[i] = _byteswap_ulong(data[i]);
        }
    }

    return true;
}

void BinLoader::LoadAnimations(std::string const& path, Model& model)
{
    std::filesystem::path folderPath(path);
    folderPath = folderPath.parent_path();
    folderPath /= "Animation";
    if (std::filesystem::exists(folderPath) && std::filesystem::is_directory(folderPath))
    {
        for (const auto& entry : std::filesystem::directory_iterator(folderPath))
        {
            if (entry.is_regular_file() && entry.path().extension() == ".anim")
            {
                ReadAnimation(model, entry);
            }
        }
    }
}

bool BinLoader::ReadString(std::ifstream& file, std::string& str)
{
    uint32_t size;

    if (!Read32Bits(file, size)) return false;

    str.resize(size);

    if (!file.read(str.data(), size)) return false;

    return true;
}

bool BinLoader::Read32Bits(std::ifstream& file, uint32_t& val)
{
    if (!file.read((char*)&val, 4)) return false;

    if (IsBigEndian())
    {
        val = _byteswap_ulong(val);
    }

    return true;
}


void BinLoader::WriteHeader(std::ofstream& file)
{
    // Header
    MeshHeader header;
    header.m_magic = 0x4D455348; // "MESH"
    header.m_version = CURRENT_VERSION;
    header.m_vertexSize = sizeof(Apex::Rendering::Vertex);
    header.m_keyframeSize = sizeof(KeyFrame);

    if (IsBigEndian())
    {
        header.m_magic = _byteswap_ulong(header.m_magic);
        header.m_version = _byteswap_ulong(header.m_version);
        header.m_vertexSize = _byteswap_ulong(header.m_vertexSize);
        header.m_keyframeSize = _byteswap_ulong(header.m_keyframeSize);
    }

    file.write((char*)&header, sizeof(header));
}

void BinLoader::WriteVerticesIndices(Mesh const& mesh, std::ofstream& file)
{
    // Counts
    uint32_t vertexCount = mesh.m_vertices.size();
    uint32_t indexCount = mesh.m_indices.size();

    Write32Bits(file, vertexCount);
    Write32Bits(file, indexCount);

    // Data
    // swap all byte if big endian
    if (IsBigEndian())
    {
        std::vector<Apex::Rendering::Vertex> vertices = mesh.m_vertices;
        std::vector<uint32_t> indices = mesh.m_indices;

        // only works if vertices uses only 4 bytes types
        uint32_t* data = reinterpret_cast<uint32_t*>(vertices.data());
        size_t totalU32 = (vertices.size() * sizeof(Apex::Rendering::Vertex)) / 4;

        for (size_t i = 0; i < totalU32; i++)
        {
            data[i] = _byteswap_ulong(data[i]);
        }

        file.write((char*)data, vertexCount * sizeof(Apex::Rendering::Vertex));

        data = reinterpret_cast<uint32_t*>(indices.data());

        for (size_t i = 0; i < indices.size(); i++)
        {
            data[i] = _byteswap_ulong(data[i]);
        }

        file.write((char*)data, indexCount * sizeof(uint32_t));
    }
    else
    {
        file.write((char*)mesh.m_vertices.data(), vertexCount * sizeof(Apex::Rendering::Vertex));
        file.write((char*)mesh.m_indices.data(), indexCount * sizeof(uint32_t));
    }
}

void BinLoader::WriteSkeleton(Model const& model, std::ofstream& file)
{
    uint32_t boneCount = model.m_skeleton.size();

    Write32Bits(file, boneCount);

    for (BoneInfo bone : model.m_skeleton)
    {
        uint32_t* matrix = reinterpret_cast<uint32_t*>(&bone.m_offsetMatrix[0][0]);
        if (IsBigEndian())
        {
            size_t size = sizeof(LibMath::Matrix4) / 4;
            for (size_t i = 0; i < size; i++)
            {
                matrix[i] = _byteswap_ulong(matrix[i]);
            }
        }
        file.write((char*)matrix, sizeof(LibMath::Matrix4));

        Write32Bits(file, *(reinterpret_cast<uint32_t*>(&bone.m_parentIndex)));

        uint32_t nbChild = bone.m_children.size();
        Write32Bits(file, nbChild);

        int32_t* data = bone.m_children.data();
        if (IsBigEndian())
        {
            for (size_t i = 0; i < bone.m_children.size(); i++)
            {
                data[i] = _byteswap_ulong(data[i]);
            }
        }
        file.write((char*)data, bone.m_children.size() * 4);
    }
}

void BinLoader::WriteBoneMap(Model const& model, std::ofstream& file)
{
    uint32_t count = model.m_boneMap.size();

    Write32Bits(file, count);

    for (auto const& [key, value] : model.m_boneMap)
    {
        WriteString(file, key);
        Write32Bits(file, value);
    }
}

bool BinLoader::WriteAnimations(Model const& model, std::filesystem::path const& path)
{
    size_t animCount = model.m_animations.size();

    for (size_t i = 0; i < animCount; i++)
    {
        Animation const& animation = model.m_animations[i];

        std::filesystem::path fileName = animation.m_name + ".anim";

        std::ofstream file(path / fileName, std::ios::binary);

        if (!file.is_open())
        {
            LOG_ERROR_CAT("Resource", "Failed to open file '{}' for writing", path.string());
            return false;
        }

        uint32_t count = animation.m_boneAnimations.size();

        Write32Bits(file, count);

        for (size_t j = 0; j < count; j++)
        {
            WriteSingleBoneAnimation(animation.m_boneAnimations[j], file);
        }
    }

    return true;
}

void BinLoader::WriteSingleBoneAnimation(SingleBoneAnimation const& sba, std::ofstream& file)
{
    uint32_t count = sba.m_frames.size();

    Write32Bits(file, count);

    // Data
    // swap all byte if big endian
    if (IsBigEndian())
    {
        std::vector<KeyFrame> frames = sba.m_frames;

        // only works if vertices uses only 4 bytes types
        uint32_t* data = reinterpret_cast<uint32_t*>(frames.data());
        size_t totalU32 = (frames.size() * sizeof(KeyFrame)) / 4;

        for (size_t i = 0; i < totalU32; i++)
        {
            data[i] = _byteswap_ulong(data[i]);
        }

        file.write((char*)data, count * sizeof(KeyFrame));
    }
    else
    {
        file.write((char*)sba.m_frames.data(), count * sizeof(KeyFrame));
    }
}

void BinLoader::WriteString(std::ofstream& file, std::string const& str)
{
    uint32_t size = (uint32_t)str.size();

    Write32Bits(file, size);

    file.write(str.data(), size);
}

void BinLoader::Write32Bits(std::ofstream& file, uint32_t const& val)
{
    uint32_t writeVal = val;

    if (IsBigEndian())
    {
        writeVal = _byteswap_ulong(writeVal);
    }

    file.write((char*)&writeVal, sizeof(writeVal));
}