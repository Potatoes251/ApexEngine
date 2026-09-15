#ifndef SOUND_PLAYER
#define SOUND_PLAYER

#include "UI.h"
#include "Audio.h"
#include "Window.h"

#include <filesystem>
#include <string>
#include <vector>

namespace Fs = std::filesystem;
using namespace Apex::UserInterface;

namespace Apex::Editor
{
    class SoundPlayer
    {
    public:
        explicit SoundPlayer(IGUI* gui, Apex::Audio::IAudioEngine* audio);
        ~SoundPlayer();
        SoundPlayer(const SoundPlayer&) = delete;
        SoundPlayer& operator=(const SoundPlayer&) = delete;

        void Open(const Fs::path& path, int instanceIndex = 0);
        void Close();
        void Focus();
        bool IsOpen()             const { return m_open; }
        const Fs::path& GetPath() const { return m_path; }

        void Draw();

    private:
        void DrawToolbar();
        void DrawWaveformPanel();
        void DrawInfoPanel();

        void LoadAudio(const Fs::path& path);
        void UnloadAudio();
        void DecodeWaveform();

        std::string FormatTime(float seconds) const;
        std::string FileSizeString()          const;

        IGUI* m_gui;
        Apex::Audio::IAudioEngine* m_audio;

        bool        m_open = false;
        bool        m_pendingFocus = false;
        int         m_instanceIndex = 0;
        Fs::path    m_path;
        std::string m_filename;
        uintmax_t   m_fileBytes = 0;

        Apex::Audio::TrackedSound m_handle{};
        float m_volume = 1.f;
        bool  m_loop = false;

        std::vector<float> m_waveform;
        static constexpr int   WAVEFORM_SAMPLES = 512;
        static constexpr float INFO_HEIGHT = 90.f;
    };
}

#endif