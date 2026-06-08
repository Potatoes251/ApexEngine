#include "IrrKlangAudio.h"

#include "Log.h"

#include <irrKlang.h>   // the only file that touches irrKlang
#include <iostream>
#include <fstream>
#include <cassert>

namespace Apex::Audio
{
    // ── Factory ─────────────────────────────────────────────────────
    IAudioEngine* CreateAudioEngine()
    {
        IAudioEngine* engine = new IrrKlangAudio();
        return engine;
    }

    IrrKlangAudio::IrrKlangAudio()
    {
        m_engine = irrklang::createIrrKlangDevice();
        if (!m_engine)
        {
            LOG_ERROR("[Audio] Failed to create irrKlang device");
        }

        m_channelVolumes.resize((size_t)AudioChannel::Count, 1);
    }

    // ── Destructor ───────────────────────────────────────────────────
    IrrKlangAudio::~IrrKlangAudio()
    {
        if (m_engine)
        {
            StopAll();
            m_engine->drop();
        }
    }

    // ── Global controls ──────────────────────────────────────────────
    void  IrrKlangAudio::SetMasterVolume(float volume)
    {
        if (m_engine) m_engine->setSoundVolume(volume);
    }

    void IrrKlangAudio::SetChannelVolume(AudioChannel channel, float volume)
    {
        m_channelVolumes[(int)channel] = volume;

        for (auto& sound : m_channelSounds[(int)channel])
            GetSound(sound)->setVolume(sound.m_baseVolume * volume);
    }

    float IrrKlangAudio::GetMasterVolume() const
    {
        return m_engine ? m_engine->getSoundVolume() : 0.f;
    }

    float IrrKlangAudio::GetChannelVolume(AudioChannel channel) const
    {
        return m_channelVolumes.at((int)channel);
    }

    void IrrKlangAudio::SetAllPaused(bool paused)
    {
        if (m_engine) m_engine->setAllSoundsPaused(paused);
    }

    void IrrKlangAudio::StopAll()
    {
        if (m_engine) m_engine->stopAllSounds();
        for (auto& [handle, sound] : m_sounds)
            if (sound) sound->drop();
        m_sounds.clear();
    }

    // ── 2-D playback ─────────────────────────────────────────────────
    TrackedSound IrrKlangAudio::Play2D(const std::string& filePath,
        const PlayOptions& options)
    {
        if (!m_engine) return {};

        irrklang::ISound* sound = m_engine->play2D(
            filePath.c_str(),
            options.m_loop,
            options.m_startPaused,
            true
        );

        if (!sound) return {};

        sound->setVolume(options.m_volume * m_channelVolumes[(int)options.m_channel]);
        return RegisterSound(sound, options.m_volume);
    }

    // ── 3-D positional playback ───────────────────────────────────────
    TrackedSound IrrKlangAudio::Play3D(const std::string& filePath,
                                      const LibMath::Vector3& position,
                                      const PlayOptions& options)
    {
        if (!m_engine) return {};

        irrklang::vec3df pos(position[0], position[1], position[2]);

        irrklang::ISound* sound = m_engine->play3D(
            filePath.c_str(),
            pos,
            options.m_loop,
            options.m_startPaused,
            true
        );

        if (!sound) return {};

        sound->setVolume(options.m_volume * m_channelVolumes[(int)options.m_channel]);
        return RegisterSound(sound);
    }

    // ── Per-instance controls ─────────────────────────────────────────
    void IrrKlangAudio::StopSound(TrackedSound handle)
    {
        irrklang::ISound* sound = GetSound(handle);
        if (!sound) return;

        sound->stop();
        sound->drop();
        m_sounds.erase(handle.m_sound);
    }

    void IrrKlangAudio::PauseSound(TrackedSound handle, bool paused)
    {
        irrklang::ISound* sound = GetSound(handle);
        if (sound) sound->setIsPaused(paused);
    }

    void IrrKlangAudio::SetVolume(TrackedSound handle, float volume)
    {
        irrklang::ISound* sound = GetSound(handle);
        if (sound) sound->setVolume(volume);
    }

    bool IrrKlangAudio::IsPlaying(TrackedSound handle) const
    {
        irrklang::ISound* sound = GetSound(handle);
        return sound && !sound->isFinished() && !sound->getIsPaused();
    }

    bool IrrKlangAudio::IsPaused(TrackedSound handle) const
    {
        irrklang::ISound* sound = GetSound(handle);
        return sound && !sound->isFinished() && sound->getIsPaused();
    }

    float IrrKlangAudio::GetPlayPosition(TrackedSound handle) const
    {
        irrklang::ISound* sound = GetSound(handle);
        if (!sound) return 0.f;
        return static_cast<float>(sound->getPlayPosition()) / 1000.f; // divide by 1000 because it's in milliseconds
    }

    float IrrKlangAudio::GetDuration(TrackedSound handle) const
    {
        irrklang::ISound* sound = GetSound(handle);
        if (!sound) return 0.f;
        irrklang::ISoundSource* source = sound->getSoundSource();
        if (!source) return 0.f;
        return static_cast<float>(source->getPlayLength()) / 1000.f; // divide by 1000 because it's in milliseconds
    }

    std::vector<float> IrrKlangAudio::GetSampleData(const std::string& filePath) const
    {
        if (!m_engine) return {};

        irrklang::ISoundSource* source = m_engine->getSoundSource(filePath.c_str(), true);
        if (!source) return {};

        irrklang::SAudioStreamFormat format = source->getAudioFormat();
        irrklang::ik_s16* raw = static_cast<irrklang::ik_s16*>(source->getSampleData());
        if (!raw) return {};

        int totalSamples = format.FrameCount * format.ChannelCount;
        std::vector<float> result;
        result.reserve(totalSamples / format.ChannelCount);

        for (int i = 0; i < totalSamples; i += format.ChannelCount)
        {
            // Convert 16-bit signed integer (-32768 to 32767) to float range (-1.0 to 1.0).
            // 32768 is 2^15, the standard divisor for normalizing 16-bit PCM audio.
            result.push_back(raw[i] / 32768.f);
        }
            

        return result;
    }

    // ── Listener ─────────────────────────────────────────────────────
    void IrrKlangAudio::SetListenerPosition(const LibMath::Vector3& position,
                                            const LibMath::Vector3& lookDirection,
                                            const LibMath::Vector3& up)
    {
        if (!m_engine) return;

        m_engine->setListenerPosition(
            irrklang::vec3df(position[0], position[1], position[2]),
            irrklang::vec3df(lookDirection[0], lookDirection[1], lookDirection[2]),
            irrklang::vec3df(0.f, 0.f, 0.f),  // velocity (Doppler) — zero for now
            irrklang::vec3df(up[0], up[1], up[2])
        );
    }

    // ── Preload ───────────────────────────────────────────────────────
    void IrrKlangAudio::Preload(const std::string& filePath)
    {
        if (m_engine)
        {
            m_engine->addSoundSourceFromFile(filePath.c_str());
        }
            
    }

    // ── Private helpers ───────────────────────────────────────────────
    TrackedSound IrrKlangAudio::RegisterSound(irrklang::ISound* sound, float volume)
    {
        TrackedSound handle = { m_nextHandle++, volume };
        m_sounds[handle.m_sound] = sound;
        return handle;
    }

    irrklang::ISound* IrrKlangAudio::GetSound(TrackedSound handle) const
    {
        auto it = m_sounds.find(handle.m_sound);
        return (it != m_sounds.end()) ? it->second : nullptr;
    }

    void IrrKlangAudio::PruneFinished()
    {
        for (auto it = m_sounds.begin(); it != m_sounds.end(); )
        {
            if (!it->second || it->second->isFinished())
            {
                if (it->second) it->second->drop();
                it = m_sounds.erase(it);
            }
            else ++it;
        }
    }

} // namespace Apex::Audio