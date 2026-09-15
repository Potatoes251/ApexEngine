#include "AudioComponent.h"

#include "Object.h"

#include "Application.h"

using namespace Apex::Audio;

AudioComponent::~AudioComponent()
{
	Stop();
}

void AudioComponent::OnStart()
{
	m_engine = Application::Get()->GetAudio();
	if (!m_sound.empty())
		m_engine->Preload(m_sound); // loads into cache, no playback
}

void AudioComponent::Play()
{
	m_soundHandle = m_engine->Play2D(m_sound, { m_loop, m_volume });
}

void AudioComponent::Play3D()
{
	Object* owner = GetOwner();
	
	if (!owner) return;

	m_soundHandle = m_engine->Play3D(m_sound, owner->GetGlobalPosition(), { m_loop, m_volume });
}

void AudioComponent::PlayAt(LibMath::Vector3 pos)
{
	m_soundHandle = m_engine->Play3D(m_sound, pos, { m_loop, m_volume });
}

void AudioComponent::Stop()
{
	if (m_engine)
		m_engine->StopSound(m_soundHandle);
}

void AudioComponent::Serialize(std::ostream& out) const
{
	out << "        \"sound\": \"" << m_sound << "\",\n";
	out << "        \"volume\": " << m_volume << ",\n";
	out << "        \"channel\": " << m_channel << ",\n";
	out << "        \"loop\": " << m_loop << "\n";
}

std::vector<Apex::ExposedVar> AudioComponent::GetExposedVariables()
{
	return { 
		{ "Sound", ExposedVar::String, &m_sound }, 
		{ "Volume", ExposedVar::Float, &m_volume }, 
		{ "Channel", ExposedVar::Enum, &m_channel, { "SFX", "Music" } },
		{ "Loop", ExposedVar::Bool, &m_loop },
	};
}
