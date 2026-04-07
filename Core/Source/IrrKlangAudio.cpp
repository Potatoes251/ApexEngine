#include "IrrKlangAudio.h"

#include <irrKlang.h>   // the only file that touches irrKlang
#include <iostream>

namespace Apex::Audio
{
    // ── Factory ─────────────────────────────────────────────────────
    IAudioEngine* CreateAudioEngine()
    {
        IAudioEngine* engine = new IrrKlangAudio();
        if (!engine->Initialize())
        {
            delete engine;
            return nullptr;
        }
        return engine;
    }

    // ── Destructor ───────────────────────────────────────────────────
    IrrKlangAudio::~IrrKlangAudio()
    {
        Shutdown();
    }

    // ── Lifecycle ────────────────────────────────────────────────────
    bool IrrKlangAudio::Initialize()
    {
        m_engine = irrklang::createIrrKlangDevice();
        if (!m_engine)
        {
            std::cout << "[Audio] Failed to create irrKlang device.\n";
            return false;
        }
        return true;
    }

    void IrrKlangAudio::Shutdown()
    {
        // Drop all tracked sound instances first.
        for (auto& [handle, sound] : m_sounds)
        {
            if (sound)
            {
                sound->stop();
                sound->drop();
            }
        }
        m_sounds.clear();

        if (m_engine)
        {
            m_engine->drop();
            m_engine = nullptr;
        }
    }

    // ── Global controls ──────────────────────────────────────────────
    void  IrrKlangAudio::SetMasterVolume(float volume)
    {
        if (m_engine) m_engine->setSoundVolume(volume);
    }

    float IrrKlangAudio::GetMasterVolume() const
    {
        return m_engine ? m_engine->getSoundVolume() : 0.f;
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
    SoundHandle IrrKlangAudio::Play2D(const std::string& filePath,
        const PlayOptions& options)
    {
        if (!m_engine) return INVALID_SOUND;

		const std::string path = filePath;

        irrklang::ISound* sound = m_engine->play2D(
            path.c_str(),
            options.m_loop,
            options.m_startPaused,
            true
        );

        if (!sound) return INVALID_SOUND;

        sound->setVolume(options.m_volume);
        return RegisterSound(sound);
    }

    // ── 3-D positional playback ───────────────────────────────────────
    SoundHandle IrrKlangAudio::Play3D(const std::string& filePath,
                                      const LibMath::Vector3& position,
                                      const PlayOptions& options)
    {
        if (!m_engine) return INVALID_SOUND;

        irrklang::vec3df pos(position[0], position[1], position[2]);

		const std::string path = filePath;

        irrklang::ISound* sound = m_engine->play3D(
            path.c_str(),
            pos,
            options.m_loop,
            options.m_startPaused,
            true
        );

        if (!sound) return INVALID_SOUND;

        sound->setVolume(options.m_volume);
        return RegisterSound(sound);
    }

    // ── Per-instance controls ─────────────────────────────────────────
    void IrrKlangAudio::StopSound(SoundHandle handle)
    {
        irrklang::ISound* sound = GetSound(handle);
        if (!sound) return;

        sound->stop();
        sound->drop();
        m_sounds.erase(handle);
    }

    void IrrKlangAudio::PauseSound(SoundHandle handle, bool paused)
    {
        irrklang::ISound* sound = GetSound(handle);
        if (sound) sound->setIsPaused(paused);
    }

    void IrrKlangAudio::SetVolume(SoundHandle handle, float volume)
    {
        irrklang::ISound* sound = GetSound(handle);
        if (sound) sound->setVolume(volume);
    }

    bool IrrKlangAudio::IsPlaying(SoundHandle handle) const
    {
        irrklang::ISound* sound = GetSound(handle);
        return sound && !sound->isFinished() && !sound->getIsPaused();
    }

    bool IrrKlangAudio::IsPaused(SoundHandle handle) const
    {
        irrklang::ISound* sound = GetSound(handle);
        return sound && !sound->isFinished() && sound->getIsPaused();
    }

    float IrrKlangAudio::GetPlayPosition(SoundHandle handle) const
    {
        irrklang::ISound* sound = GetSound(handle);
        if (!sound) return 0.f;
        return static_cast<float>(sound->getPlayPosition()) / 1000.f; // divide by 1000 because it's in milliseconds
    }

    float IrrKlangAudio::GetDuration(SoundHandle handle) const
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
			const std::string path = "Assets/Sounds/" + filePath;
            m_engine->addSoundSourceFromFile(path.c_str());
        }
            
    }

    // ── Private helpers ───────────────────────────────────────────────
    SoundHandle IrrKlangAudio::RegisterSound(irrklang::ISound* sound)
    {
        SoundHandle handle = m_nextHandle++;
        m_sounds[handle] = sound;
        return handle;
    }

    irrklang::ISound* IrrKlangAudio::GetSound(SoundHandle handle) const
    {
        auto it = m_sounds.find(handle);
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