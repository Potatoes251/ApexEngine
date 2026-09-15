#include "Console.h"

#include "Log.h"

#include <cassert>
#include <algorithm>
#include <unordered_map>

using namespace Apex::UserInterface;

void Apex::Console::Add(LogEntry entry)
{
	m_collapsedDirty = true;

	std::lock_guard<std::mutex> lock(m_logMutex);

	if (m_logs.empty())
	{
		m_logs.push_back(entry);
		return;
	}

	LogEntry& last = m_logs.back();
	if (last.m_message == entry.m_message
		&& last.m_category == entry.m_category
		&& last.m_level == entry.m_level)
	{
		last.m_count++;
		return;
	}

	m_logs.push_back(entry);
}

void Apex::Console::Draw()
{
	m_gui->BeginPanel("Console", nullptr, true, false, false);
	DrawToolbar();

	if (m_collapseAll && m_collapsedDirty)
		CollapseLogs();

	if (m_collapseAll) 
	{
		DrawLogs(m_collapsedLogs);
	}
	else 
	{
		std::lock_guard<std::mutex> lock(m_logMutex);
		DrawLogs(m_logs);
	}

	m_gui->EndPanel();
}

void Apex::Console::DrawToolbar()
{
	m_gui->PushStyleVariable(StyleVariable::FramePadding, 8.f, 5.f);
	m_gui->PushFont(FontID::Default);

	if (m_gui->Button("-"))
		m_fontSize = (FontID)std::max((int)m_fontSize - 1, 0);

	m_gui->SameLine();
	if (m_gui->Button("+"))
		m_fontSize = (FontID)std::min((int)m_fontSize + 1, int(FontID::Count) - 1);

	m_gui->SameLine(0.f, 16.f);
	if (m_gui->Button("clear"))
	{
		std::lock_guard<std::mutex> lock(m_logMutex);
		m_logs.clear();
		m_collapsedDirty = true;
	}

	m_gui->SameLine(0.f, 16.f);
	m_gui->VerticalSeparator();
	m_gui->SameLine(0.f, 16.f);
	bool show = m_level & LogLevel::Info;
	if (m_gui->Checkbox("Info", &show))
	{
		if (show)
			m_level |= LogLevel::Info;
		else
			m_level &= ~LogLevel::Info;
	}

	show = m_level & LogLevel::Warning;
	m_gui->SameLine(0.f, 16.f);
	if (m_gui->Checkbox("Warning", &show))
	{
		if (show)
			m_level |= LogLevel::Warning;
		else
			m_level &= ~LogLevel::Warning;
	}

	show = m_level & LogLevel::Error;
	m_gui->SameLine(0.f, 16.f);
	if (m_gui->Checkbox("Error", &show))
	{
		if (show)
			m_level |= LogLevel::Error;
		else
			m_level &= ~LogLevel::Error;
	}

	show = m_level & LogLevel::Debug;
	m_gui->SameLine(0.f, 16.f);
	if (m_gui->Checkbox("Debug", &show))
	{
		if (show)
			m_level |= LogLevel::Debug;
		else
			m_level &= ~LogLevel::Debug;
	}

	m_gui->SameLine(0.f, 16.f);
	m_gui->VerticalSeparator();
	m_gui->SameLine(0.f, 16.f);
	m_gui->Checkbox("Category", &m_showCategory);
	m_gui->SameLine(0.f, 16.f);
	m_gui->Checkbox("Collapse", &m_collapseAll);

	m_gui->SameLine(0.f, 16.f);
	m_gui->SetNextItemWidth(220.f);
	m_gui->InputText("##console_search", m_searchBuffer, 256, "Search logs...");

	m_gui->PopFont();
	m_gui->PopStyleVariable();
}

void Apex::Console::DrawLogs(std::vector<LogEntry> const& logs)
{
	m_gui->BeginChildPanel("##logs", 0.f, 0.f, false);

	std::string search = m_searchBuffer;

	m_gui->PushFont((FontID)m_fontSize);
	for (auto& log : logs)
	{
		if (m_level & log.m_level)
		{
			std::string str = log.m_message;
			if (!search.empty())
			{
				search = ToLower(search);
				std::string msg = ToLower(log.m_message);
				std::string cat = ToLower(log.m_category);
				if (msg.find(search) == std::string::npos
					&& cat.find(search) == std::string::npos)
					continue;
			}

			if (m_showCategory)
				str = '[' + log.m_category + "] " + str;

			if (log.m_count > 1)
				str += "(x" + std::to_string(log.m_count) + ')';

			m_gui->TextColored(GetColorFromLevel(log), str);
		}
	}
	m_gui->PopFont();

	m_gui->EndChildPanel();
}

void Apex::Console::CollapseLogs()
{
	m_collapsedLogs.clear();

	std::unordered_map<std::string, LogEntry> map;

	{
		std::lock_guard<std::mutex> lock(m_logMutex);
		for (const LogEntry& log : m_logs)
		{
			std::string key = log.m_category + "|" + log.m_message + "|" + std::to_string((int)log.m_level);

			auto it = map.find(key);
			if (it == map.end())
			{
				map[key] = log;
			}
			else
			{
				map[key].m_count++;
			}
		}
	}

	for (auto& [_, entry] : map)
		m_collapsedLogs.push_back(entry);

	m_collapsedDirty = false;
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
		return { 1.f, 1.f, 1.f, 1.f };
	}
}

std::string Apex::Console::ToLower(std::string str)
{
	std::transform(str.begin(), str.end(), str.begin(), ::tolower);
	return str;
}
