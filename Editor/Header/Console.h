#ifndef CONSOLE
#define CONSOLE

#include "ISink.h"

#include <vector>

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

		static UserInterface::Color GetColorFromLevel(LogEntry const& entry);

		static std::string ToLower(std::string str);

		char m_searchBuffer[256] = {};

		std::vector<LogEntry> m_logs;

		UserInterface::IGUI* m_gui;

		UserInterface::FontID m_fontSize = UserInterface::FontID::Default;

		uint8_t m_level = LogLevel::Warning | LogLevel::Error;
		bool	m_showCategory = true;
	};
}


#endif // !CONSOLE

