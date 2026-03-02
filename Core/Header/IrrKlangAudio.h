#ifndef IRRKLANG_AUDIO
#define IRRKLANG_AUDIO

// ================================================================================
// IrrKlangAudio.h — INTERNAL. Do NOT include outside the audio implementation.
// ================================================================================

#include "Audio.h"

#include <unordered_map>

// Forward-declare irrKlang types — keeps this header clean.
namespace irrklang
{
    class ISoundEngine;
    class ISound;
}

namespace Apex::Audio
{
    class IrrKlangAudio final : public IAudioEngine
    {
    public:
        IrrKlangAudio() = default;
        ~IrrKlangAudio() override;

        IrrKlangAudio(const IrrKlangAudio&) = delete;
        IrrKlangAudio& operator=(const IrrKlangAudio&) = delete;

        bool Initialize() override;
        void Shutdown()   override;

        void  SetMasterVolume(float volume)  override;
        float GetMasterVolume() const        override;
        void  SetAllPaused(bool paused)      override;
        void  StopAll()                      override;

        SoundHandle Play2D(const std::string& filePath,
                           const PlayOptions& options) override;

        SoundHandle Play3D(const std::string& filePath,
                           const LibMath::Vector3& position,
                           const PlayOptions& options) override;

        void  StopSound(SoundHandle handle)              override;
        void  PauseSound(SoundHandle handle, bool paused) override;
        void  SetVolume(SoundHandle handle, float volume) override;
        bool  IsPlaying(SoundHandle handle) const        override;

        void SetListenerPosition(const LibMath::Vector3& position,
                                 const LibMath::Vector3& lookDirection,
                                 const LibMath::Vector3& up) override;

        void Preload(const std::string& filePath) override;

    private:
        irrklang::ISoundEngine* m_engine = nullptr;
        SoundHandle             m_nextHandle = 1;

        // Maps engine-side handles to live irrKlang ISound* instances.
        // Entries are pruned on StopSound and lazily on IsPlaying queries.
        std::unordered_map<SoundHandle, irrklang::ISound*> m_sounds;

        SoundHandle             RegisterSound(irrklang::ISound* sound);
        irrklang::ISound*       GetSound(SoundHandle handle) const;
        void                    PruneFinished();  // remove stopped sounds from the map
    };

} // namespace Apex::Audio

#endif // IRRKLANG_AUDIO