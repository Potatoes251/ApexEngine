#ifndef EDITOR_ICON
#define EDITOR_ICON

// ============================================================
// EditorIcon.h - Loads PNG icons for use in ImageButton calls.
//
// Icons are cached by path after first load so they are only
// uploaded to the GPU once per editor session.
//
// Usage:
//   uint32_t id = EditorIcon::Get("EngineAssets/Icons/save.png");
//   if (m_gui.ImageButton("##save", id, {24, 24})) { ... }
//
// All textures are freed by EditorIcon::Shutdown() which should
// be called before the GL context is destroyed.
// ============================================================

#include <string>
#include <unordered_map>

namespace Apex::Editor
{
    class EditorIcon
    {
    public:
        // Returns the GL texture ID for the icon at path.
        // Loads and caches on first call. Returns 0 on failure.
        static uint32_t Get(const std::string& path);

        // Free all cached textures. Call before GL context teardown.
        static void Shutdown();

    private:
        static uint32_t Load(const std::string& path);

        static inline std::unordered_map<std::string, uint32_t> m_cache;
    };
}

#endif