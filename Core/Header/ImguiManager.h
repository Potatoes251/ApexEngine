#ifndef IMGUI_MANAGER
#define IMGUI_MANAGER

#include "UI.h"
#include <vector>
//#include "IconsFontAwesome.h"

using std::vector;

// Forward declaration of IMGUI types to avoid including IMGUI headers in UI.h
struct ImGUIManager;
struct ImFont;

namespace Apex::UserInterface
{
	namespace  FontSize
	{
		constexpr 	float	SMALL = 14.0f;
		constexpr 	float	DEFAULT = 18.0f;
		constexpr 	float	LARGE = 28.0f;
		constexpr 	float	TITLE = 36.0f;
	};

	struct FontInitializer
	{
		FontInitializer(FontID id, const std::string& file, float fontSize);

		FontID      	m_id;
		std::string		m_file;
		float			m_fontSize;
	};

	std::vector<FontInitializer> m_fontDefinitions =
	{
		{ FontID::Small,   "../../assets/Fonts/Roboto-Regular.ttf", FontSize::SMALL },
		{ FontID::Default, "../../assets/Fonts/Roboto-Regular.ttf", FontSize::DEFAULT },
		{ FontID::Large,   "../../assets/Fonts/Roboto-Bold.ttf",    FontSize::LARGE },
		{ FontID::Title,   "../../assets/Fonts/Roboto-Bold.ttf",    FontSize::TITLE }
	};

	class ImGUIManager final : public IGUI
	{
	public:
		ImGUIManager(GLFWwindow* nativeWindow);
		~ImGUIManager() override;

		void	Initialize() override;
		void	Shutdown() override;

		void	LoadFonts() override;

		void	BeginFrame() override;
		void	EndFrame() override;

		bool	WantCaptureMouse() const override;
		bool	WantCaptureKeyboard() const override;

		void	BeginPanel(const std::string& title) override;
		void	EndPanel() override;

		//In checkbox
		void	Text(const std::string& text) override;
		bool	Button(const std::string& label) override;
		bool	Checkbox(const std::string& label, bool* v) override;
		bool	SliderFloat(const char* label, float* v, float min, float max) override;
		bool	SliderInt(const char* label, int* v, int min, int max) override;
		bool	ColorEdit3(const char* label, float color[3]) override;
		void	Separator() override;
		void	SameLine() override;

		//Out of checkbox
		void	DrawText2D(const std::string& text, float x, float y, float size, FontID id) override;

	private:
		GLFWwindow*		m_window = nullptr;

		vector<ImFont*> m_fonts;
	};
}

#endif // !IMGUI_MANAGER