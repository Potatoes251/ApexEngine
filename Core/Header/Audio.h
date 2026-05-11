#ifndef AUDIO
#define AUDIO

// ============================================================
// Audio.h — Public audio interface.
// irrKlang headers are NEVER referenced here.
// This is the only header the rest of the engine should include.
// ============================================================

#include <string>
#include <vector>

#include "LibMath/Vector/Vector3.h"

namespace Apex::Audio
{
    // ── Sound handle ─────────────────────────────────────────────
    // Opaque handle representing a playing sound instance.
    // Lifetime is managed by the engine — never delete directly.
    using SoundHandle = unsigned int;
    constexpr SoundHandle INVALID_SOUND = 0;

    enum class AudioChannel
    {
        SFX,
        Music,
        Count,
    };

    struct TrackedSound
    {
        SoundHandle m_sound = INVALID_SOUND;
        float       m_baseVolume = 1.f;
    };

    // ── Playback options ──────────────────────────────────────────
    struct PlayOptions
    {
        bool  m_loop = false;
        float m_volume = 1.f;   // 0.0 – 1.0
        bool  m_startPaused = false;
        AudioChannel m_channel;
    };

    // ── IAudioEngine ─────────────────────────────────────────────
    class IAudioEngine
    {
    public:
        virtual ~IAudioEngine() = default;

        // Global controls
        virtual void   SetMasterVolume(float volume) = 0;  // 0.0 – 1.0
        virtual void   SetChannelVolume(AudioChannel channel, float volume) = 0;
        virtual float  GetMasterVolume() const = 0;
        virtual float  GetChannelVolume(AudioChannel channel) const = 0;
        virtual void   SetAllPaused(bool paused) = 0;
        virtual void   StopAll() = 0;

        // 2-D (non-positional) playback
        virtual TrackedSound Play2D(const std::string& filePath,
                                   const PlayOptions& options = {}) = 0;

        // 3-D (positional) playback
        virtual TrackedSound Play3D(const std::string& filePath,
                                   const LibMath::Vector3& position,
                                   const PlayOptions& options = {}) = 0;
            
        // Per-instance controls (no-op on INVALID_SOUND)
        virtual void  StopSound(TrackedSound handle) = 0;
        virtual void  PauseSound(TrackedSound handle, bool paused) = 0;
        virtual void  SetVolume(TrackedSound handle, float volume) = 0;
        virtual bool  IsPlaying(TrackedSound handle) const = 0;
        virtual bool  IsPaused(TrackedSound handle) const = 0;

        virtual float GetPlayPosition(TrackedSound handle) const = 0;
        virtual float GetDuration(TrackedSound handle) const = 0;

        // Returns normalized [-1, 1] PCM samples for the given file.
        // irrKlang decodes all supported formats (OGG, WAV, MP3, FLAC...).
        // Returns empty vector if the file cannot be decoded.
        virtual std::vector<float> GetSampleData(const std::string& filePath) const = 0;

        // Listener transform (for 3-D audio)
        virtual void SetListenerPosition(const LibMath::Vector3& position,
                                         const LibMath::Vector3& lookDirection,
                                         const LibMath::Vector3& up = { 0.f, 1.f, 0.f }) = 0;

        // Pre-cache a sound file so first play has no hitch
        virtual void Preload(const std::string& filePath) = 0;
    };

    // ── Factory ───────────────────────────────────────────────────
    IAudioEngine* CreateAudioEngine();

} // namespace Apex::Audio

#endif // AUDIO