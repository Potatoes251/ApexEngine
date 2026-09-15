#ifndef CONSOLE
#define CONSOLE

#include "ISink.h"

#include <vector>
#include <mutex>


#include "UI.h"

namespace Apex
{
	namespace UserInterface { class IGUI; }

	// editor console
	class Console : public ISink
	{
	public:
		Console(UserInterface::IGUI* gui) : m_gui(gui) {}
		Console(Console const&) = delete;
		Console& operator=(Console const&) = delete;
		~Console() = default;

		void Add(LogEntry entry) override;

		void Draw();

	private:
		void DrawToolbar();
		void DrawLogs(std::vector<LogEntry> const& logs);

		void CollapseLogs();

		static UserInterface::Color GetColorFromLevel(LogEntry const& entry);

		static std::string ToLower(std::string str);

		char m_searchBuffer[256] = {};

		std::mutex m_logMutex;

		std::vector<LogEntry> m_logs;
		std::vector<LogEntry> m_collapsedLogs;

		UserInterface::IGUI* m_gui;

		UserInterface::FontID m_fontSize = UserInterface::FontID::Default;

		uint8_t m_level = LogLevel::Warning | LogLevel::Error;
		bool	m_showCategory = true;
		bool	m_collapseAll = false;
		bool	m_collapsedDirty = false;
	};
}


#endif // !CONSOLE

