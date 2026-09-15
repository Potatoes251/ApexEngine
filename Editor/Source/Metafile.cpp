#include "MetaFile.h"

#include <fstream>
#include <sstream>

namespace Apex::Editor
{
    // Static helpers

    Fs::path MetaFile::SidecarPath(const Fs::path& assetPath)
    {
        return Fs::path(assetPath.string() + ".meta");
    }

    bool MetaFile::Exists(const Fs::path& assetPath)
    {
        return Fs::exists(SidecarPath(assetPath));
    }

    // Load 

    bool MetaFile::Load(const Fs::path& assetPath)
    {
        m_entries.clear();
        m_dirty = false;

        std::ifstream file(SidecarPath(assetPath));
        if (!file.is_open())
            return false;   // not an error � file simply doesn't exist yet

        std::string line;
        while (std::getline(file, line))
        {
            // Strip carriage return (Windows line endings)
            if (!line.empty() && line.back() == '\r')
                line.pop_back();

            // Skip blank lines and comments
            if (line.empty() || line[0] == '#')
                continue;

            auto separator = line.find('=');
            if (separator == std::string::npos)
                continue;   // malformed line � skip silently

            std::string key = line.substr(0, separator);
            std::string value = line.substr(separator + 1);
            m_entries[key] = value;
        }

        return true;
    }

    // Save

    bool MetaFile::Save(const Fs::path& assetPath) const
    {
        std::ofstream file(SidecarPath(assetPath));
        if (!file.is_open())
            return false;

        file << "# Apex Engine meta file - do not edit manually\n";

        for (const auto& [key, value] : m_entries)
            file << key << '=' << value << '\n';

        return true;
    }

    // Key access 

    bool MetaFile::Has(const std::string& key) const
    {
        return m_entries.count(key) > 0;
    }

    std::string MetaFile::Get(const std::string& key, const std::string& defaultVal) const
    {
        auto it = m_entries.find(key);
        return (it != m_entries.end()) ? it->second : defaultVal;
    }

    int MetaFile::GetInt(const std::string& key, int defaultVal) const
    {
        auto it = m_entries.find(key);
        if (it == m_entries.end()) return defaultVal;
        try { return std::stoi(it->second); }
        catch (...) { return defaultVal; }
    }

    bool MetaFile::GetBool(const std::string& key, bool defaultVal) const
    {
        auto it = m_entries.find(key);
        if (it == m_entries.end()) return defaultVal;
        return it->second == "true";
    }

    // Setters 

    void MetaFile::Set(const std::string& key, const std::string& value)
    {
        m_entries[key] = value;
        m_dirty = true;
    }

    void MetaFile::Set(const std::string& key, int value)
    {
        Set(key, std::to_string(value));
    }

    void MetaFile::Set(const std::string& key, bool value)
    {
        Set(key, std::string(value ? "true" : "false"));
    }

    void MetaFile::Remove(const std::string& key)
    {
        if (m_entries.erase(key) > 0)
            m_dirty = true;
    }
}