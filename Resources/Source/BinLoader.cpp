#include "BinLoader.h"

#include "Log.h"

#include <fstream>
#include <filesystem>

bool BinLoader::Load(std::string path, Mesh& mesh, std::string& sourcePath_out)
{
    std::ifstream file(path, std::ios::binary);

    if (!file.is_open())
    {
        LOG_ERROR("Failed to open bin file : {}", path);
        return false;
    }

    file.seekg(0, std::ios::end);
    std::streamsize size = file.tellg();

    if (size == 0)
    {
        LOG_ERROR("File is empty : {}", path);
        return false;
    }

    file.seekg(0, std::ios::beg);

    if (!ReadHeader(file, sourcePath_out)
        || !ReadVerticesIndices(mesh, file)
        || !ReadBoneInfo(mesh, file)
        || !ReadBoneMap(mesh, file))
        return false;

    std::filesystem::path folderPath(path);
    folderPath = folderPath.parent_path();
    folderPath /= "Animation";
    if (std::filesystem::exists(folderPath) && std::filesystem::is_directory(folderPath)) 
    {
        for (const auto& entry : std::filesystem::directory_iterator(folderPath)) 
        {
            if (entry.is_regular_file() && entry.path().extension() == ".anim")
            {
                ReadAnimation(mesh, entry);
            }
        }
    }

    return true;
}

bool BinLoader::Load(std::string path, Animation& animation)
{
    std::ifstream file(path, std::ios::binary);

    if (!file.is_open())
    {
        LOG_ERROR("Failed to open file '{}' for reading", path);
        return false;
    }

    animation.m_name = path;

    uint32_t count;

    if (!Read32Bits(file, count)) return false;

    animation.m_boneAnimations.resize(count);

    for (size_t j = 0; j < count; j++)
    {
        if (!ReadSingleBoneAnimation(animation.m_boneAnimations[j], file)) return false;
    }

    return true;
}

bool BinLoader::Export(std::string path, Mesh const& mesh)
{
    std::filesystem::path binaryPath = path;
    binaryPath.replace_extension(".mesh");
    std::filesystem::path folderPath = binaryPath.parent_path();
    folderPath /= binaryPath.stem();

    if (!std::filesystem::exists(folderPath))
    {
        std::filesystem::create_directory(folderPath);
    }

    std::ofstream file(folderPath / binaryPath.filename(), std::ios::binary);

    if (!file.is_open())
    {
        LOG_ERROR("Failed to open file '{}' for writing", path);
        return false;
    }

    WriteHeader(file);
    WriteString(file, path);
    WriteVerticesIndices(mesh, file);
    WriteBoneInfo(mesh, file);
    WriteBoneMap(mesh, file);

    if (mesh.m_animations.size() == 0) return true;

    folderPath /= "Animation";
    if (!std::filesystem::exists(folderPath)) 
    {
        std::filesystem::create_directory(folderPath);
    }

    WriteAnimations(mesh, folderPath);

    return true;
}


bool BinLoader::ReadHeader(std::ifstream& file, std::string& sourcePath_out)
{
    MeshHeader header;

    if (!Read32Bits(file, header.m_magic)) return false;
    if (!Read32Bits(file, header.m_version)) return false;
    if (!Read32Bits(file, header.m_vertexSize)) return false;
    if (!Read32Bits(file, header.m_keyframeSize)) return false;
    if (!Read32Bits(file, header.m_boneinfoSize)) return false;
    if (!ReadString(file, sourcePath_out))
    {
        sourcePath_out.clear();
        return false;
    }

    if (header.m_magic != 0x4D455348)
    {
        LOG_ERROR("Invalid file format");
        return false;
    }
    if (header.m_version != CURRENT_VERSION)
    {
        LOG_WARNING("File needs to be updated (version was {})", header.m_version);
        return false;
    }
    if (header.m_vertexSize != sizeof(Apex::Rendering::Vertex)
        || header.m_keyframeSize != sizeof(KeyFrame)
        || header.m_boneinfoSize != sizeof(BoneInfo))
    {
        LOG_WARNING("Struct size doesnt match");
        return false;
    }

    return false;
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
    if constexpr (IsBigEndian())
    {
        uint32_t* data = reinterpret_cast<uint32_t*>(indices.data());

        for (size_t i = 0; i < indices.size(); i++)
        {
            data[i] = _byteswap_ulong(data[i]);
        }

        // only works if vertices uses only 4 bytes types
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

bool BinLoader::ReadBoneInfo(Mesh& mesh, std::ifstream& file)
{
    uint32_t boneCount;

    if (!Read32Bits(file, boneCount)) return false;

    std::vector<BoneInfo> bones(boneCount);

    if (!file.read((char*)bones.data(), boneCount * sizeof(BoneInfo)))
        return false;

    // swap all byte if big endian
    if constexpr (IsBigEndian())
    {
        uint32_t* data = reinterpret_cast<uint32_t*>(bones.data());
        size_t totalU32 = (bones.size() * sizeof(BoneInfo)) / 4;

        for (size_t i = 0; i < totalU32; i++)
        {
            data[i] = _byteswap_ulong(data[i]);
        }
    }

    mesh.m_bones = bones;

    return true;
}

bool BinLoader::ReadBoneMap(Mesh& mesh, std::ifstream& file)
{
    uint32_t count;

    if (!Read32Bits(file, count)) return false;

    mesh.m_boneMap.reserve(count);

    for (uint32_t i = 0; i < count; i++)
    {
        std::string key;
        uint32_t value;

        if (!ReadString(file, key)) return false;
        if (!Read32Bits(file, value)) return false;

        mesh.m_boneMap.emplace(std::move(key), value);
    }

    return true;
}

bool BinLoader::ReadAnimation(Mesh& mesh, std::filesystem::path const& path)
{
    std::ifstream file(path, std::ios::binary);

    if (!file.is_open())
    {
        LOG_ERROR("Failed to open file '{}' for reading", path.string());
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

    mesh.m_animations.push_back(animation);

    return true;
}

bool BinLoader::ReadSingleBoneAnimation(SingleBoneAnimation& sba, std::ifstream& file)
{
    uint32_t count;

    if (!Read32Bits(file, count)) return false;

    sba.m_frames.resize(count);

    if (!file.read((char*)sba.m_frames.data(), count * sizeof(KeyFrame))) return false;

    if constexpr (IsBigEndian())
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

    if constexpr (IsBigEndian())
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
    header.m_boneinfoSize = sizeof(BoneInfo);

    if constexpr (IsBigEndian())
    {
        header.m_magic = _byteswap_ulong(header.m_magic);
        header.m_version = _byteswap_ulong(header.m_version);
        header.m_vertexSize = _byteswap_ulong(header.m_vertexSize);
        header.m_keyframeSize = _byteswap_ulong(header.m_keyframeSize);
        header.m_boneinfoSize = _byteswap_ulong(header.m_boneinfoSize);
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
    if constexpr (IsBigEndian())
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

void BinLoader::WriteBoneInfo(Mesh const& mesh, std::ofstream& file)
{
    uint32_t boneCount = mesh.m_bones.size();

    Write32Bits(file, boneCount);

    // Data
    // swap all byte if big endian
    if constexpr (IsBigEndian())
    {
        std::vector<BoneInfo> bones = mesh.m_bones;

        // only works if bones uses only 4 bytes types
        uint32_t* data = reinterpret_cast<uint32_t*>(bones.data());
        size_t totalU32 = (bones.size() * sizeof(BoneInfo)) / 4;

        for (size_t i = 0; i < totalU32; i++)
        {
            data[i] = _byteswap_ulong(data[i]);
        }

        file.write((char*)data, boneCount * sizeof(BoneInfo));
    }
    else
    {
        file.write((char*)mesh.m_bones.data(), boneCount * sizeof(BoneInfo));
    }
}

void BinLoader::WriteBoneMap(Mesh const& mesh, std::ofstream& file)
{
    uint32_t count = mesh.m_boneMap.size();

    Write32Bits(file, count);

    for (auto const& [key, value] : mesh.m_boneMap)
    {
        WriteString(file, key);
        Write32Bits(file, value);
    }
}

bool BinLoader::WriteAnimations(Mesh const& mesh, std::filesystem::path const& path)
{
    size_t animCount = mesh.m_animations.size();

    for (size_t i = 0; i < animCount; i++)
    {
        Animation const& animation = mesh.m_animations[i];

        std::filesystem::path fileName = animation.m_name + ".anim";

        std::ofstream file(path / fileName, std::ios::binary);

        if (!file.is_open())
        {
            LOG_ERROR("Failed to open file '{}' for writing", path.string());
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
    if constexpr (IsBigEndian())
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

    if constexpr (IsBigEndian())
    {
        writeVal = _byteswap_ulong(writeVal);
    }

    file.write((char*)&val, sizeof(val));
}