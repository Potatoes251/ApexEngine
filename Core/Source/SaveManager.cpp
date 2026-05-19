#include "SaveManager.h"
#include "Scene.h"
#include "Log.h"

#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <cassert>

namespace Apex::Save
{
    // =========================================================================
    //  Construction
    // =========================================================================

    SaveManager::SaveManager(const std::string& saveDir)
        : m_saveDir(saveDir)
    {
        std::filesystem::create_directories(m_saveDir);
    }

    // =========================================================================
    //  Public API
    // =========================================================================

    bool SaveManager::Save(SaveData data, uint32_t slotIndex)
    {
        data.slotIndex = slotIndex;

        const std::string path = BuildPath(slotIndex);

        std::ofstream file(path);
        if (!file.is_open())
        {
            LOG_ERROR("SaveManager::Save – could not open '{}' for writing", path);
            return false;
        }

        file << Serialize(data);

        LOG_INFO("SaveManager::Save – slot {} written to '{}'", slotIndex, path);
        return true;
    }

    std::optional<SaveData> SaveManager::Load(uint32_t slotIndex) const
    {
        const std::string path = BuildPath(slotIndex);

        std::ifstream file(path);
        if (!file.is_open())
        {
            LOG_WARNING("SaveManager::Load – slot {} not found ('{}')", slotIndex, path);
            return std::nullopt;
        }

        std::string json((std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>());

        auto data = Deserialize(json);
        if (!data)
            LOG_ERROR("SaveManager::Load – failed to parse slot {} ('{}')", slotIndex, path);

        return data;
    }

    bool SaveManager::Delete(uint32_t slotIndex) const
    {
        const std::string path = BuildPath(slotIndex);
        std::error_code ec;
        bool removed = std::filesystem::remove(path, ec);
        if (removed)
            LOG_INFO("SaveManager::Delete – slot {} deleted", slotIndex);
        return removed;
    }

    std::vector<SaveManager::SlotInfo> SaveManager::ListSlots() const
    {
        std::vector<SlotInfo> slots;

        for (const auto& entry : std::filesystem::directory_iterator(m_saveDir))
        {
            if (entry.path().extension() != ".sav")
                continue;

            std::ifstream file(entry.path());
            if (!file.is_open()) continue;

            std::string json((std::istreambuf_iterator<char>(file)),
                std::istreambuf_iterator<char>());

            auto data = Deserialize(json);
            if (!data) continue;

            slots.push_back({ data->slotIndex, data->scenePath, data->score });
        }

        std::sort(slots.begin(), slots.end(),
            [](const SlotInfo& a, const SlotInfo& b) { return a.slotIndex < b.slotIndex; });

        return slots;
    }

    void SaveManager::ApplyToScene(const SaveData& data, Rendering::Scene* scene) const
    {
        if (!scene) return;

        // Destroy every object whose ID appears in the collected-items list.
        // This ensures items the player already picked up don't reappear.
        for (const CollectedItem& item : data.collectedItems)
        {
            if (scene->GetObjectWithId(item.objectId))
            {
                scene->DestroyObject(item.objectId);
                LOG_INFO("SaveManager::ApplyToScene – removed collected item {}",
                    item.objectId);
            }
        }
    }

    // =========================================================================
    //  Private helpers
    // =========================================================================

    std::string SaveManager::BuildPath(uint32_t slotIndex) const
    {
        return m_saveDir + "slot_" + std::to_string(slotIndex) + ".sav";
    }

    // =========================================================================
    //  Serialization  (hand-written JSON – no external library required)
    // =========================================================================

    std::string SaveManager::EscapeString(const std::string& s)
    {
        std::string out;
        out.reserve(s.size() + 2);
        for (char c : s)
        {
            if (c == '"')  out += "\\\"";
            else if (c == '\\') out += "\\\\";
            else if (c == '\n') out += "\\n";
            else if (c == '\r') out += "\\r";
            else if (c == '\t') out += "\\t";
            else                out += c;
        }
        return out;
    }

    std::string SaveManager::Serialize(const SaveData& data)
    {
        std::ostringstream o;
        o << std::fixed;
        o.precision(6);

        o << "{\n";
        o << "  \"slot\":      " << data.slotIndex << ",\n";
        o << "  \"scene\":     \"" << EscapeString(data.scenePath) << "\",\n";
        o << "  \"score\":     " << data.score << ",\n";
        o << "  \"hp\":     " << data.hp << ",\n";
        o << "  \"position\":  ["
            << data.playerPosition[0] << ","
            << data.playerPosition[1] << ","
            << data.playerPosition[2] << "],\n";

        o << "  \"collected\": [\n";
        for (size_t i = 0; i < data.collectedItems.size(); ++i)
        {
            const CollectedItem& item = data.collectedItems[i];
            o << "    { \"id\": " << item.objectId << "\" }";
            if (i + 1 < data.collectedItems.size()) o << ",";
            o << "\n";
        }
        o << "  ]\n";
        o << "}\n";

        return o.str();
    }

    // ---- Tiny pull-parser (covers the subset we write above) ----------------

    // Returns the raw value string for a top-level key, e.g. "42" or "\"foo\"".
    std::string SaveManager::FindValue(const std::string& json, const std::string& key)
    {
        std::string needle = "\"" + key + "\"";
        size_t pos = json.find(needle);
        if (pos == std::string::npos) return {};

        size_t colon = json.find(':', pos + needle.size());
        if (colon == std::string::npos) return {};

        size_t start = colon + 1;
        while (start < json.size() && (json[start] == ' ' || json[start] == '\t')) ++start;

        // Find the end of the value (next comma or closing brace at depth 0)
        size_t end = start;
        if (json[start] == '"')
        {
            // String value – walk to closing quote respecting escapes
            ++end;
            while (end < json.size() && !(json[end] == '"' && json[end - 1] != '\\'))
                ++end;
            ++end; // include closing quote
        }
        else if (json[start] == '[')
        {
            // Array – walk to matching ]
            int depth = 0;
            while (end < json.size())
            {
                if (json[end] == '[') ++depth;
                else if (json[end] == ']') { --depth; if (depth == 0) { ++end; break; } }
                ++end;
            }
        }
        else
        {
            // Number / bool / null
            while (end < json.size() && json[end] != ',' && json[end] != '\n'
                && json[end] != '}' && json[end] != ']')
                ++end;
        }

        std::string val = json.substr(start, end - start);
        // Trim trailing whitespace
        while (!val.empty() && (val.back() == ' ' || val.back() == '\t')) val.pop_back();
        return val;
    }

    // Returns raw object strings from a JSON array with the given key.
    std::vector<std::string> SaveManager::FindArray(const std::string& json, const std::string& key)
    {
        std::string needle = "\"" + key + "\"";
        size_t pos = json.find(needle);
        if (pos == std::string::npos) return {};

        size_t bracket = json.find('[', pos);
        if (bracket == std::string::npos) return {};

        // Collect objects between '[' and matching ']'
        std::vector<std::string> items;
        size_t i = bracket + 1;
        int depth = 1;
        size_t objStart = std::string::npos;

        while (i < json.size() && depth > 0)
        {
            char c = json[i];
            if (c == '[') { ++depth; }
            else if (c == ']') { --depth; if (depth == 0) break; }
            else if (c == '{' && depth == 1)
            {
                objStart = i;
                int d2 = 0;
                while (i < json.size())
                {
                    if (json[i] == '{') ++d2;
                    else if (json[i] == '}') { --d2; if (d2 == 0) { items.push_back(json.substr(objStart, i - objStart + 1)); break; } }
                    ++i;
                }
            }
            ++i;
        }

        return items;
    }

    std::optional<SaveData> SaveManager::Deserialize(const std::string& json)
    {
        try
        {
            SaveData data;

            auto slotStr = FindValue(json, "slot");
            auto tsStr = FindValue(json, "timestamp");
            auto scoreStr = FindValue(json, "score");
            auto livesStr = FindValue(json, "lives");
            auto sceneVal = FindValue(json, "scene");
            auto posVal = FindValue(json, "position");

            if (slotStr.empty() || tsStr.empty() || sceneVal.empty())
                return std::nullopt;

            data.slotIndex = static_cast<uint32_t>(std::stoul(slotStr));
            data.score = scoreStr.empty() ? 0 : std::stoi(scoreStr);

            // Strip surrounding quotes from the scene string
            if (sceneVal.size() >= 2 && sceneVal.front() == '"')
                data.scenePath = sceneVal.substr(1, sceneVal.size() - 2);

            // Parse position array "[x,y,z]"
            if (!posVal.empty() && posVal.front() == '[')
            {
                std::string inner = posVal.substr(1, posVal.size() - 2);
                std::istringstream ss(inner);
                std::string tok;
                float components[3] = {};
                int idx = 0;
                while (std::getline(ss, tok, ',') && idx < 3)
                    components[idx++] = std::stof(tok);
                data.playerPosition = { components[0], components[1], components[2] };
            }

            // Parse collected items
            for (const std::string& obj : FindArray(json, "collected"))
            {
                auto idStr = FindValue(obj, "id");
                auto tagVal = FindValue(obj, "tag");

                if (idStr.empty()) continue;

                CollectedItem item;
                item.objectId = std::stoull(idStr);

                data.collectedItems.push_back(std::move(item));
            }

            return data;
        }
        catch (const std::exception& ex)
        {
            LOG_ERROR("SaveManager::Deserialize – exception: {}", ex.what());
            return std::nullopt;
        }
    }
}