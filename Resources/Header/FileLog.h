#ifndef FILE_LOG
#define FILE_LOG

#include "ISink.h"

#include <fstream>
#include <mutex>

namespace Apex
{
	class FileLog : public ISink
	{
	public:
		FileLog(std::string path);
		~FileLog();

		void Add(LogEntry entry) override;

	private:
		void AddLogLevel(LogEntry entry);
		void CleanupLogs();

		static inline const std::string m_logDir = "Logs";
		static inline const std::string m_oldLogDir = "Logs/Archive";
		static constexpr int MAX_LOGS = 20;

		std::mutex m_logMutex;
		std::ofstream m_currentLog;
		std::ofstream m_latestLog;
	};
}

#endif // !FILE_LOG

