#include "Console.h"

#include "Log.h"

#include <cassert>

using namespace Apex::UserInterface;

void Apex::Console::Add(LogEntry entry)
{
	m_logs.push_back(entry);
}

void Apex::Console::Draw()
{
	m_gui->BeginPanel("Console", nullptr, true, false, false);
	DrawToolbar();
	
	m_gui->BeginChildPanel("##logs", 0.f, 0.f, false);

	Color color = {1,1,1,1};
	m_gui->PushFont((FontID)m_fontSize);
	for (auto& log : m_logs)
	{
		if (m_flag & log.m_level)
			m_gui->TextColored(GetColorFromLevel(log), log.m_message);
	}
	m_gui->PopFont();


	m_gui->EndChildPanel();
	m_gui->EndPanel();
}

void Apex::Console::DrawToolbar()
{
	if (m_gui->Button("-"))
	{
		m_fontSize = (FontID)std::max((int)m_fontSize - 1, 0);
	}
	m_gui->SameLine();
	if (m_gui->Button("+"))
		m_fontSize = (FontID)std::min((int)m_fontSize + 1, int(FontID::Count) - 1);

	bool show = m_flag & LogLevel::Info;
	m_gui->SameLine();
	if (m_gui->Checkbox("Info", &show))
	{
		if (show)
			m_flag |= LogLevel::Info;
		else
			m_flag &= ~LogLevel::Info;
	}

	show = m_flag & LogLevel::Warning;
	m_gui->SameLine();
	if (m_gui->Checkbox("Warning", &show))
	{
		if (show)
			m_flag |= LogLevel::Warning;
		else
			m_flag &= ~LogLevel::Warning;
	}

	show = m_flag & LogLevel::Error;
	m_gui->SameLine();
	if (m_gui->Checkbox("Error", &show))
	{
		if (show)
			m_flag |= LogLevel::Error;
		else
			m_flag &= ~LogLevel::Error;
	}

	show = m_flag & LogLevel::Debug;
	m_gui->SameLine();
	if (m_gui->Checkbox("Debug", &show))
	{
		if (show)
			m_flag |= LogLevel::Debug;
		else
			m_flag &= ~LogLevel::Debug;
	}
}

Color Apex::Console::GetColorFromLevel(LogEntry const& entry)
{
	switch (entry.m_level)
	{
	case LogLevel::Info:	return { .85f, .85f, .85f, 1.f};
	case LogLevel::Warning:	return { .95f, .75f, .1f, 1.f };
	case LogLevel::Error:	return { 1.f, .0f, .0f, 1.f };
	case LogLevel::Debug:	return { .5f, .5f, .5f, 1.f };
	default:
		assert(false && "Unhandled case of LogLevel in Console::GetColorFromLevel");
		break;
	}
}
