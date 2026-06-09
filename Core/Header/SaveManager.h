#ifndef SAVE_MANAGER
#define SAVE_MANAGER

#include "SaveData.h"
#include "Scene.h"

#include <string>
#include <vector>
#include <optional>

namespace Apex::Save
{
    // -----------------------------------------------------------------
    // SaveManager
    //
    // Owns the save-slot directory and handles all serialization.
    // Designed to live inside Application (or Game) as a unique_ptr.
    //
    // File format  : JSON  (hand-written, no third-party dependency)
    // Default dir  : "Saves/"
    // File naming  : "Saves/slot_0.sav", "Saves/slot_1.sav", …
    //
    // Typical usage
    //   // saving
    //   SaveData data;
    //   data.scenePath      = m_app->GetScenePath();
    //   data.playerPosition = player->GetPosition();
    //   data.score          = player->GetScore();
    //   data.lives          = player->GetLives();
    //   for (auto id : player->GetCollectedIds())
    //       data.collectedItems.push_back({ id, "coin" });
    //   m_saveManager->Save(data, 0);
    //
    //   // loading
    //   if (auto d = m_saveManager->Load(0))
    //   {
    //       m_app->LoadScene(d->scenePath, false);
    //       // restore player from *d …
    //       m_app->GetSaveManager()->ApplyCollectedItems(*d, m_app->GetScene());
    //       m_app->GetScene()->Start();
    //   }
    // -----------------------------------------------------------------
    class SaveManager
    {
    public:
        // Returns metadata for every slot file that exists on disk,
        // sorted by slot index. Useful for a load-game UI.
        struct SlotInfo
        {
            uint32_t    m_slotIndex;
            std::string m_scenePath;
            int         m_score;
        };
        
        explicit SaveManager(const std::string& saveDir = "Saves/");

        std::vector<SlotInfo> ListSlots() const;

        // Returns the save-directory path this manager was constructed with.
        const std::string& GetSaveDir() const { return m_saveDir; }

        // --- Persistence ------------------------------------------------

        // Write slot to disk. Returns true on success.
        bool Save(SaveData data, uint32_t slotIndex);

        // Read slot from disk. Returns nullopt when file is missing/corrupt.
        std::optional<SaveData> Load(uint32_t slotIndex) const;

        // Delete a save slot file from disk.
        bool Delete(uint32_t slotIndex) const;

        // --- Scene integration ------------------------------------------

        // After calling Application::LoadScene() with the saved scene path,
        // call this to remove collected items from the freshly-loaded scene.
        // Forward-declare Scene so callers don't need to include Scene.h here.
        void ApplyToScene(const SaveData& data, Rendering::Scene* scene) const;

        std::string BuildPath(uint32_t slotIndex) const;

    private:

        // --- Serialization helpers (hand-written JSON) ------------------
        static std::string Serialize(const SaveData& data);
        static std::optional<SaveData> Deserialize(const std::string& json);

        static std::string EscapeString(const std::string& s);

        std::string m_saveDir;
    };
}

#endif SAVE_MANAGER