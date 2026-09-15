#ifndef ISINK
#define ISINK

#include <string>

namespace Apex
{
	enum LogLevel : uint8_t
	{
		Info = 1 << 0,
		Warning = 1 << 1,
		Error = 1 << 2,
		Debug = 1 << 3
	};

	struct LogEntry
	{
		std::string m_message;
		std::string m_category;
		LogLevel	m_level;
		int			m_count = 1;
	};

	class ISink
	{
	public:
		virtual void Add(LogEntry entry) = 0;
	};
}

#endif // !ISINK

