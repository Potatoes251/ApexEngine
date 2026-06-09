#include "SaveManager.h"
#include "Scene.h"
#include "Log.h"
#include "SerializationParser.h"

#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <cassert>

using namespace Apex::Serialization;

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
        data.m_slotIndex = slotIndex;

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

            slots.push_back({ data->m_slotIndex, data->m_scenePath, data->m_score });
        }

        std::sort(slots.begin(), slots.end(),
            [](const SlotInfo& a, const SlotInfo& b) { return a.m_slotIndex < b.m_slotIndex; });

        return slots;
    }

    void SaveManager::ApplyToScene(const SaveData& data, Rendering::Scene* scene) const
    {
        if (!scene) return;

        // Destroy every object whose ID appears in the collected-items list.
        // This ensures items the player already picked up don't reappear.
        for (const CollectedItem& item : data.m_collectedItems)
        {
            if (scene->GetObjectWithId(item.m_objectId))
            {
                scene->DestroyObject(item.m_objectId);
                LOG_INFO("SaveManager::ApplyToScene – removed collected item {}",
                    item.m_objectId);
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
        o << "  \"slot\":      " << data.m_slotIndex << ",\n";
        o << "  \"scene\":     \"" << EscapeString(data.m_scenePath) << "\",\n";
        o << "  \"score\":     " << data.m_score << ",\n";
        o << "  \"hp\":     " << data.m_hp << ",\n";
        o << "  \"position\":  ["
            << data.m_playerPosition[0] << ","
            << data.m_playerPosition[1] << ","
            << data.m_playerPosition[2] << "],\n";

        o << "  \"collected\": [";

        for (size_t i = 0; i < data.m_collectedItems.size(); ++i)
        {
            o << data.m_collectedItems[i].m_objectId;

            if (i + 1 < data.m_collectedItems.size())
                o << ", ";
        }

        o << "]\n";
        o << "}\n";

        return o.str();
    }

    std::optional<SaveData> SaveManager::Deserialize(const std::string& json)
    {
        try
        {
            SerialParser parser(json);

            SaveData data;

            parser.Expect('{');

            while (!parser.Peek('}'))
            {
                std::string key = parser.ParseString();
                parser.Expect(':');

                if (key == "slot")
                {
                    data.m_slotIndex = parser.ParseInt();
                }
                else if (key == "scene")
                {
                    data.m_scenePath = parser.ParseString();
                }
                else if (key == "score")
                {
                    data.m_score = parser.ParseInt();
                }
                else if (key == "hp")
                {
                    data.m_hp = parser.ParseInt();
                }
                else if (key == "position")
                {
                    data.m_playerPosition = parser.ParseVector3();
                }
                else if (key == "collected")
                {
                    parser.Expect('[');

                    while (!parser.Peek(']'))
                    {
                        CollectedItem item;
                        item.m_objectId = static_cast<size_t>(parser.ParseInt());
                        data.m_collectedItems.push_back(item);

                        if (parser.Peek(','))
                            parser.Expect(',');
                    }

                    parser.Expect(']');
                }
                else
                {
                    parser.ParseFloat();
                }

                if (parser.Peek(','))
                    parser.Expect(',');
            }

            parser.Expect('}');

            return data;
        }
        catch (...)
        {
            LOG_ERROR("SaveManager::Deserialize – parsing failed");
            return std::nullopt;
        }
    }
}