#ifndef IRRKLANG_AUDIO
#define IRRKLANG_AUDIO

// ================================================================================
// IrrKlangAudio.h - INTERNAL. Do NOT include outside the audio implementation.
// ================================================================================

#include "Audio.h"

#include <unordered_map>

// Forward-declare irrKlang types - keeps this header clean.
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
        IrrKlangAudio();
        ~IrrKlangAudio() override;

        IrrKlangAudio(const IrrKlangAudio&) = delete;
        IrrKlangAudio& operator=(const IrrKlangAudio&) = delete;

        void  SetMasterVolume(float volume)  override;
        void  SetChannelVolume(AudioChannel channel, float volume) override;
        float GetMasterVolume() const        override;
        float GetChannelVolume(AudioChannel channel) const override;
        void  SetAllPaused(bool paused)      override;
        void  StopAll()                      override;

        TrackedSound Play2D(const std::string& filePath,
                           const PlayOptions& options) override;

        TrackedSound Play3D(const std::string& filePath,
                           const LibMath::Vector3& position,
                           const PlayOptions& options) override;

        void  StopSound(TrackedSound handle)              override;
        void  PauseSound(TrackedSound handle, bool paused) override;
        void  SetVolume(TrackedSound handle, float volume) override;
        bool  IsPlaying(TrackedSound handle) const        override;
		bool  IsPaused(TrackedSound handle) const         override;

        float GetPlayPosition(TrackedSound handle) const override;
        float GetDuration(TrackedSound handle) const override;
        std::vector<float> GetSampleData(const std::string& filePath) const override;

        void SetListenerPosition(const LibMath::Vector3& position,
                                 const LibMath::Vector3& lookDirection,
                                 const LibMath::Vector3& up) override;

        void Preload(const std::string& filePath) override;

    private:
        TrackedSound            RegisterSound(irrklang::ISound* sound, float volume = 1.f);
        irrklang::ISound*       GetSound(TrackedSound handle) const;
        void                    PruneFinished();  // remove stopped sounds from the map

        irrklang::ISoundEngine* m_engine = nullptr;
        SoundHandle             m_nextHandle = 1;

        // Maps engine-side handles to live irrKlang ISound* instances.
        // Entries are pruned on StopSound and lazily on IsPlaying queries.
        std::unordered_map<SoundHandle, irrklang::ISound*> m_sounds;

        std::vector<float> m_channelVolumes;
        std::vector<std::vector<TrackedSound>> m_channelSounds;
    };

} // namespace Apex::Audio

#endif // IRRKLANG_AUDIO