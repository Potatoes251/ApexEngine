#ifndef SAVE_DATA
#define SAVE_DATA

#include "LibMath/Vector/Vector3.h"

#include <string>
#include <vector>
#include <cstdint>
#include <ctime>

namespace Apex::Save
{
    // -----------------------------------------------------------------
    // One entry in the collected-items list.
    // An item is identified by its scene object ID so that Application
    // can remove it from the scene on load.
    // -----------------------------------------------------------------
    struct CollectedItem
    {
        size_t      m_objectId = 0;       // ID of the Object that was picked up
    };

    // -----------------------------------------------------------------
    // Everything the save system writes to disk and reads back.
    // Add new fields freely — SaveManager handles serialization.
    // -----------------------------------------------------------------
    struct SaveData
    {
        // --- Meta -------------------------------------------------------
        uint32_t    m_slotIndex = 0;

        // --- World state ------------------------------------------------
        std::string m_scenePath;             // path that Application::LoadScene() accepts

        // --- Player state -----------------------------------------------
        LibMath::Vector3 m_playerPosition;
        int         m_score = 0;
        int         m_hp = 100;

        // --- Collected items --------------------------------------------
        // Objects in this list are removed from the scene after loading.
        std::vector<CollectedItem> m_collectedItems;
    };
}

#endif // SAVE_DATA