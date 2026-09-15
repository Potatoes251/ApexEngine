#include "FileLog.h"

#include <iomanip>
#include <ctime>
#include <sstream>
#include <filesystem>

Apex::FileLog::FileLog(std::string path)
{
	std::filesystem::create_directories(m_logDir);
	std::filesystem::create_directories(m_oldLogDir);

	m_currentLog.open("Logs/Archive/" + path);
	m_latestLog.open("Logs/latest.log");

	CleanupLogs();
}

Apex::FileLog::~FileLog()
{
	m_currentLog.close();
	m_latestLog.close();
}

void Apex::FileLog::Add(LogEntry entry)
{
	std::lock_guard<std::mutex> lock(m_logMutex);
	m_currentLog << '[' << entry.m_category << ']';
	m_latestLog << '[' << entry.m_category << ']';

	AddLogLevel(entry);

	m_currentLog << entry.m_message << std::endl;
	m_latestLog << entry.m_message << std::endl;
}

void Apex::FileLog::AddLogLevel(LogEntry entry)
{
	switch (entry.m_level)
	{
	case LogLevel::Info:
		m_currentLog << "[Info] ";
		m_latestLog << "[Info] ";
		break;
	case LogLevel::Warning:
		m_currentLog << "[Warning] ";
		m_latestLog << "[Warning] ";
		break;
	case LogLevel::Error:
		m_currentLog << "[Error] ";
		m_latestLog << "[Error] ";
		break;
	case LogLevel::Debug:
		m_currentLog << "[Debug] ";
		m_latestLog << "[Debug] ";
		break;
	default:
		break;
	}
}

void Apex::FileLog::CleanupLogs()
{
	std::vector<std::filesystem::directory_entry> files;

	for (const auto& entry : std::filesystem::directory_iterator(m_oldLogDir))
	{
		if (entry.is_regular_file() && entry.path().extension() == ".log")
		{
			files.push_back(entry);
		}
	}

	if (files.size() <= MAX_LOGS) return;

	std::sort(files.begin(), files.end(),
		[](const std::filesystem::directory_entry& a, const std::filesystem::directory_entry& b)
		{
			return std::filesystem::last_write_time(a) > std::filesystem::last_write_time(b);
		});

	for (size_t i = MAX_LOGS; i < files.size(); ++i)
	{
		std::filesystem::remove(files[i]);
	}
}