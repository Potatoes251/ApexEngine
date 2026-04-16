#ifndef LOGGER
#define LOGGER

#include "ISink.h"

#include <string>
#include <vector>
#include <memory>
#include <format>


namespace Apex
{
	class LogSystem
	{
	public:

		template<typename... Args>
		static void Log(LogLevel lvl, std::string cat, std::string_view fmt, Args&&... args);

		static void Log(LogLevel lvl, std::string cat, std::string msg);

		static void AddSink(ISink* sink);

	private:
		static inline std::vector<ISink*> m_sinks;
	};

	template<typename ...Args>
	inline void LogSystem::Log(LogLevel lvl, std::string cat, std::string_view fmt, Args&& ...args)
	{
		std::string msg = std::vformat(fmt, std::make_format_args(args...));

		Log(lvl, cat, msg);
	}
}

#endif // !LOGGER

