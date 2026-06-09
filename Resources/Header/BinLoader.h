#ifndef BIN_LOADER
#define BIN_LOADER

#include <filesystem>
#include <string>
#include <bit>

#include "Mesh.h"
#include "Model.h"

// to be increased after each modification to the bin loader
constexpr uint32_t LEGACY_VERSION = 6;
constexpr uint32_t CURRENT_VERSION = 7;

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

	bool Load(std::string const& path, Model& model);
	bool LoadLegacy(std::ifstream& file, Model& model);
	bool Export(std::string const& path, Model const& model);

	static void WriteString(std::ofstream& file, std::string const& str);
	static void Write32Bits(std::ofstream& file, uint32_t const& val);

	static bool ReadString(std::ifstream& file, std::string& str);
	static bool Read32Bits(std::ifstream& file, uint32_t& val);

	static bool IsBigEndian()
	{
		volatile union {
			uint32_t i;
			char c[4];
		} val = { 0x01020304 };

		return val.c[0] == 1;
	}
private:
	// read

	bool ReadHeader(std::ifstream& file, uint32_t& version);
	bool ReadVerticesIndices(Mesh& mesh, std::ifstream& file);
	bool ReadSkeleton(Model& model, std::ifstream& file);
	bool ReadBoneMap(Model& model, std::ifstream& file);
	bool ReadAnimation(Model& model, std::filesystem::path const& path);
	bool ReadSingleBoneAnimation(SingleBoneAnimation& sba, std::ifstream& file);

	void LoadAnimations(std::string const& path, Model& model);
	// write

	void WriteHeader(std::ofstream& file);
	void WriteSkeleton(Model const& model, std::ofstream& file);
	void WriteBoneMap(Model const& model, std::ofstream& file);

	void WriteVerticesIndices(Mesh const& mesh, std::ofstream& file);
	
	bool WriteAnimations(Model const& model, std::filesystem::path const& path);
	void WriteSingleBoneAnimation(SingleBoneAnimation const& sba, std::ofstream& file);
};

#endif // !BIN_LOADER
