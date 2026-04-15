#include "LogSystem.h"

void Apex::LogSystem::Log(LogLevel lvl, std::string msg)
{
	LogEntry entry = { msg, lvl };
	for (auto& sink : m_sinks)
	{
		sink->Add(entry);
	}
}

void Apex::LogSystem::AddSink(ISink* sink)
{
	m_sinks.push_back(sink);
}
