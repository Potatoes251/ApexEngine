#ifndef AUDIO_COMPONENT
#define AUDIO_COMPONENT

#include "Component.h"

#include "Audio.h"

namespace Apex::Audio
{
	class AudioComponent : public Component
	{
	public:
		AudioComponent(IAudioEngine* audioEngine) : m_engine(audioEngine) {}
		AudioComponent(IAudioEngine* audioEngine, std::string name, int channel, float volume, bool loop) : 
			m_engine(audioEngine), m_sound(name), m_channel(channel), m_volume(volume), m_loop(loop) {}

		~AudioComponent();

		void OnStart() override;

		void Play();
		void Play3D();
		void PlayAt(LibMath::Vector3 pos);
		void Stop();
		void SetSound(const std::string& name) { m_sound = name; }
		void SetVolume(float volume) { m_volume = volume; }
		void SetChannel(int channel) { m_channel = channel; }
		void SetLoop(bool loop) { m_loop = loop; }

		void Serialize(std::ostream& out) const override;

		std::unique_ptr<Component> Clone() override { return std::make_unique<AudioComponent>(*this); }

		const char* GetTypeName() const override { return "AudioComponent"; }
		std::vector<ExposedVar> GetExposedVariables() override;

	private:
		std::string m_sound{};

		IAudioEngine* m_engine = nullptr;

		TrackedSound m_soundHandle{};

		int m_channel = 0;
		float m_volume = 1.0f;

		bool m_loop = false;

	};
}

#endif // !AUDIO_COMPONENT

