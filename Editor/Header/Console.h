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

		void Add(LogEntry entry) override;

		void Draw();

	private:
		void DrawToolbar();

		static UserInterface::Color GetColorFromLevel(LogEntry const& entry);

		std::vector<LogEntry> m_logs;

		UserInterface::IGUI* m_gui;

		UserInterface::FontID m_fontSize = UserInterface::FontID::Default;

		uint8_t m_flag = true;
	};
}


#endif // !CONSOLE

