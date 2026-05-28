#ifndef BIN_LOADER
#define BIN_LOADER

#include <filesystem>
#include <string>
#include <bit>

#include "Mesh.h"
#include "Model.h"

// to be increased after each modifictation to the bin loader
constexpr int CURRENT_VERSION = 6;

struct MeshHeader
{
	uint32_t m_magic = 0x4D455348;
	uint32_t m_version = 1;
	uint32_t m_vertexSize = 0;
	uint32_t m_keyframeSize = 0;
};


class BinLoader
{
public:
	BinLoader() = default;
	BinLoader(BinLoader const&) = delete;
	BinLoader& operator=(BinLoader const&) = delete;
	~BinLoader() = default;

	bool Load(std::string path, Mesh& mesh_out, std::string& sourcePath_out);
	bool Export(std::string path, Model const& model);

	static void WriteString(std::ofstream& file, std::string const& str);
	static void Write32Bits(std::ofstream& file, uint32_t const& val);

	static bool ReadString(std::ifstream& file, std::string& str);
	static bool Read32Bits(std::ifstream& file, uint32_t& val);

	static bool IsBigEndian()
	{
		union {
			uint32_t i;
			char c[4];
		} val = { 0x01020304 };

		return val.c[0] == 1;
	}
private:
	// read

	bool ReadHeader(std::ifstream& file, std::string& sourcePath_out);
	bool ReadVerticesIndices(Mesh& mesh, std::ifstream& file);
	bool ReadBoneInfo(Mesh& mesh, std::ifstream& file);
	bool ReadBoneMap(Mesh& mesh, std::ifstream& file);
	bool ReadAnimation(Mesh& mesh, std::filesystem::path const& path);
	bool ReadSingleBoneAnimation(SingleBoneAnimation& sba, std::ifstream& file);

	// write

	void WriteHeader(std::ofstream& file);
	void WriteSkeleton(Model const& model, std::ofstream& file);
	void WriteBoneMap(Model const& model, std::ofstream& file);

	void WriteVerticesIndices(Mesh const& mesh, std::ofstream& file);
	
	bool WriteAnimations(Model const& model, std::filesystem::path const& path);
	void WriteSingleBoneAnimation(SingleBoneAnimation const& sba, std::ofstream& file);
};

#endif // !BIN_LOADER
