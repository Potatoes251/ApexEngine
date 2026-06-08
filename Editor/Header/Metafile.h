#ifndef META_FILE
#define META_FILE

// ============================================================
// MetaFile.h - Lightweight key=value sidecar file utility.
//
// Meta files live next to their asset with a ".meta" suffix:
//   Assets/Textures/hero.png  -  Assets/Textures/hero.png.meta
//
// Format (plain text, one entry per line):
//   key=value
//   # lines starting with # are comments
//   blank lines are ignored
//
// Values are always stored and returned as strings.
// Typed helpers (GetInt, GetBool) parse on demand.
// ============================================================

#include <filesystem>
#include <unordered_map>

namespace Fs = std::filesystem;

namespace Apex::Editor
{
	class MetaFile
	{
    public:
        // Derive the sidecar path from an asset path.
        // e.g. "Assets/Textures/foo.png" - "Assets/Textures/foo.png.meta"
        static Fs::path SidecarPath(const Fs::path& assetPath);

        // Returns true if a meta file exists for the given asset.
        static bool Exists(const Fs::path& assetPath);

        // Load from the sidecar path of assetPath.
        // Returns false if the file doesn't exist (empty MetaFile, no error).
        bool Load(const Fs::path& assetPath);

        // Save to the sidecar path of assetPath.
        // Creates the file if it doesn't exist.
        bool Save(const Fs::path& assetPath) const;

        // Key access
        bool        Has(const std::string& key) const;
        std::string Get(const std::string& key, const std::string& defaultVal = "") const;
        int         GetInt(const std::string& key, int defaultVal = 0) const;
        bool        GetBool(const std::string& key, bool defaultVal = false) const;

        void Set(const std::string& key, const std::string& value);
        void Set(const std::string& key, int   value);
        void Set(const std::string& key, bool  value);
        void Remove(const std::string& key);

        bool IsDirty() const { return m_dirty; }

    private:
        void ClearDirty() { m_dirty = false; }

        std::unordered_map<std::string, std::string> m_entries;
        bool m_dirty = false;
	};
}

#endif