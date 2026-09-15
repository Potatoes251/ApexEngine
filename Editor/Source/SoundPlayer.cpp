#include "SoundPlayer.h"

#include "EditorIcon.h"

#include <sstream>
#include <iomanip>
#include <fstream>

namespace Apex::Editor
{

    SoundPlayer::SoundPlayer(IGUI* gui, Apex::Audio::IAudioEngine* audio)
        : m_gui(gui), m_audio(audio) {}

    SoundPlayer::~SoundPlayer()
    {
        UnloadAudio();
    }

    void SoundPlayer::Open(const Fs::path& path, int instanceIndex)
    {
        UnloadAudio();
        m_path = path;
        m_filename = path.filename().string();
        m_fileBytes = Fs::exists(path) ? Fs::file_size(path) : 0;
        m_instanceIndex = instanceIndex;
        m_open = true;
        m_pendingFocus = false;
        m_volume = 1.f;
        m_loop = false;
        LoadAudio(path);
        DecodeWaveform();
    }

    void SoundPlayer::Close()
    {
        UnloadAudio();
        m_open = false;
    }

    void SoundPlayer::Focus()
    {
        m_pendingFocus = true;
    }

    void SoundPlayer::Draw()
    {
        if (!m_open) return;

        m_gui->PushFont(FontID::Default);

        if (!m_gui->IsWindowDocked())
        {
            constexpr float CASCADE = 30.f;
            float offset = static_cast<float>(m_instanceIndex) * CASCADE;
            m_gui->SetNextWindowPos(LibMath::Vector2(offset + 200.f, offset + 150.f));
            m_gui->SetNextWindowSize(LibMath::Vector2(600.f, 280.f));
        }

        if (m_pendingFocus)
        {
            m_gui->SetNextWindowFocus();
            m_pendingFocus = false;
        }

        m_gui->BeginPanel("Sound Player - " + m_filename, &m_open, true, true);

        if (!m_open)
        {
            Close();
            m_gui->EndPanel();
            m_gui->PopFont();
            return;
        }

        DrawToolbar();
        m_gui->Separator();
        DrawWaveformPanel();
        m_gui->Separator();
        DrawInfoPanel();

        m_gui->EndPanel();
        m_gui->PopFont();
    }

    void SoundPlayer::DrawToolbar()
    {
        m_gui->PushStyleVariable(StyleVariable::FramePadding, 6.f, 4.f);
        bool isPlaying = m_audio->IsPlaying(m_handle);
        bool isPaused = m_audio->IsPaused(m_handle);
        auto showTooltip = [&](const char* text) 
            { 
                if (m_gui->IsItemHovered()) 
                {   m_gui->BeginTooltip(); 
                    m_gui->Text(text); 
                    m_gui->EndTooltip(); 
                } 
            };

        // Play / Pause Button 
        uint32_t icon = isPlaying ? EditorIcon::Get("ApexAssets/Icons/pause.png") :
            (isPaused ? EditorIcon::Get("ApexAssets/Icons/play_pause.png") : EditorIcon::Get("ApexAssets/Icons/play.png"));

        if (m_gui->ImageButton("##audioControl", icon, { 22.f, 22.f })) 
        {
            if (isPlaying)      m_audio->PauseSound(m_handle, true);
            else if (isPaused)  m_audio->PauseSound(m_handle, false);
            else                m_handle = m_audio->Play2D(m_path.string(), { m_loop, m_volume });
        }
        showTooltip(isPlaying ? "Pause" : (isPaused ? "Resume" : "Play"));

        // Stop Button 
        m_gui->SameLine(0.f, 4.f);
        if (m_gui->ImageButton("##stop", EditorIcon::Get("ApexAssets/Icons/stop.png"), { 22.f, 22.f })) 
        {
            m_audio->StopSound(m_handle);
            m_handle.m_sound = Apex::Audio::INVALID_SOUND;
        }
        showTooltip("Stop");

        // Settings & Info
        auto separator = [&]() 
            { 
                m_gui->SameLine(0.f, 16.f); 
                m_gui->VerticalSeparator(); 
                m_gui->SameLine(0.f, 16.f); 
            };
        separator();
        m_gui->Checkbox("Loop", &m_loop);
        separator();

        m_gui->AlignTextToFramePadding();
        m_gui->Text("Volume:");
        m_gui->SameLine(0.f, 6.f);
        m_gui->SetNextItemWidth(80.f);
        if (m_gui->SliderFloat("##vol", &m_volume, 0.f, 1.f) && m_handle.m_sound != Apex::Audio::INVALID_SOUND)
            m_audio->SetVolume(m_handle, m_volume);

        m_gui->SameLine(0.f, 16.f);
        float position = m_audio->GetPlayPosition(m_handle);
        float duration = m_audio->GetDuration(m_handle);
        std::string timeString = FormatTime(position) + " / " + FormatTime(duration);
        m_gui->AlignTextToFramePadding();
        m_gui->Text(timeString);

        m_gui->PopStyleVariable();
    }

    void SoundPlayer::DrawWaveformPanel()
    {
        LibMath::Vector2 panelPos = m_gui->GetPanelPos();
        LibMath::Vector2 availableSize = m_gui->GetAvailableSize();
        float waveHeight = availableSize[1] - INFO_HEIGHT - 8.f;
        if (waveHeight < 20.f) waveHeight = 20.f;

        m_gui->InvisibleButton("##wave", LibMath::Vector2(availableSize[0], waveHeight));
        LibMath::Vector2 waveMin = m_gui->GetItemRectMin();
        LibMath::Vector2 waveMax = m_gui->GetItemRectMax();
        float width = waveMax[0] - waveMin[0];
        float height = waveMax[1] - waveMin[1];

        IDrawList* drawList = m_gui->GetDrawList();

        drawList->DrawRectFilled(waveMin, waveMax, 0xFF1A1A1A, 4.f);

        if (m_waveform.empty())
        {
            LibMath::Vector2 center(waveMin[0] + width * 0.5f, waveMin[1] + height * 0.5f);
            drawList->DrawText(LibMath::Vector2(center[0] - 40.f, center[1] - 8.f),
                0xFF666666, "No waveform data");
        }
        else
        {
            float xStep = width / static_cast<float>(m_waveform.size());
            float midY = waveMin[1] + height * 0.5f;
            float amplitude = height * 0.45f;

            for (int i = 0; i < static_cast<int>(m_waveform.size()) - 1; ++i)
            {
                float x0 = waveMin[0] + i * xStep;
                float x1 = waveMin[0] + (i + 1) * xStep;
                float y0 = midY - m_waveform[i] * amplitude;
                float y1 = midY - m_waveform[i + 1] * amplitude;
                drawList->AddLine(LibMath::Vector2(x0, y0), LibMath::Vector2(x1, y1),
                    0xFF44AAFF, 1.f);
            }

            float position = m_audio->GetPlayPosition(m_handle);
            float duration = m_audio->GetDuration(m_handle);
            if (duration > 0.f)
            {
                float t = position / duration;
                float playX = waveMin[0] + t * width;
                drawList->AddLine(LibMath::Vector2(playX, waveMin[1]),
                    LibMath::Vector2(playX, waveMax[1]),
                    0xFFFFFFFF, 1.5f);
            }
        }
    }

    void SoundPlayer::DrawInfoPanel()
    {
        auto row = [&](const char* label, const std::string& value)
            {
                m_gui->AlignTextToFramePadding();
                m_gui->Text(label);
                m_gui->SameLine(100.f);
                m_gui->TextDisabled(value);
            };

        row("File:", m_filename);
        row("Size:", FileSizeString());

        std::string extension = m_path.extension().string();
        for (auto& c : extension) c = static_cast<char>(::toupper(c));
        row("Format:", extension.empty() ? "Unknown" : extension.substr(1));
    }

    void SoundPlayer::LoadAudio(const Fs::path& path)
    {
        m_audio->Preload(path.string());
    }

    void SoundPlayer::UnloadAudio()
    {
        if (m_handle.m_sound != Apex::Audio::INVALID_SOUND)
        {
            m_audio->StopSound(m_handle);
            m_handle.m_sound = Apex::Audio::INVALID_SOUND;
        }
        m_waveform.clear();
    }

    void SoundPlayer::DecodeWaveform()
    {
        m_waveform.clear();

        std::vector<float> pcm = m_audio->GetSampleData(m_path.string());
        if (pcm.empty()) return;

        m_waveform.resize(WAVEFORM_SAMPLES);
        int blockSize = std::max(1, static_cast<int>(pcm.size()) / WAVEFORM_SAMPLES);

        for (int i = 0; i < WAVEFORM_SAMPLES; ++i)
        {
            int   start = i * blockSize;
            int   end = std::min(start + blockSize, static_cast<int>(pcm.size()));
            float peak = 0.f;
            for (int j = start; j < end; ++j)
                peak = std::max(peak, std::abs(pcm[j]));
            m_waveform[i] = peak;
        }
    }


    std::string SoundPlayer::FormatTime(float seconds) const
    {
        if (seconds < 0.f) seconds = 0.f;
        int minutesDisplay = static_cast<int>(seconds) / 60;
        int secondsDisplay = static_cast<int>(seconds) % 60;
        char buffer[16];
        snprintf(buffer, sizeof(buffer), "%d:%02d", minutesDisplay, secondsDisplay);
        return buffer;
    }

    std::string SoundPlayer::FileSizeString() const
    {
        if (m_fileBytes >= 1024 * 1024)
            return std::to_string(m_fileBytes / (1024 * 1024)) + " MB";
        if (m_fileBytes >= 1024)
            return std::to_string(m_fileBytes / 1024) + " KB";
        return std::to_string(m_fileBytes) + " B";
    }

} // namespace Apex::Editor