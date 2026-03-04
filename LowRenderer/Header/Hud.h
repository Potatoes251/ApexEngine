#ifndef HUD
#define HUD

#include "../../Core/Header/UI.h"
#include <string>
#include <vector>
#include <memory>
#include "LibMath/Vector/Vector2.h"
#include "Texture.h"

using std::string;
using std::vector;
using std::unique_ptr;
using LibMath::Vector2;
//using namespace Apex::UserInterface;
using Apex::UserInterface::FontID;

namespace Apex::UserInterface		// can't be 'HUD' as used on top
{
	struct UIElement
	{
	public:
				UIElement(Vector2 position, float size);
		virtual ~UIElement() = default;

		virtual	void	Draw(IGUI& gui) = 0;

		Vector2		m_position;
		float		m_size;
	};

	struct UIText : UIElement
	{
	public:
		UIText(string text, Vector2 position, float size, FontID font);

		void	Draw(IGUI& gui) override;

		string	m_text;
		FontID	m_font;
	};
	
	struct UIImage : UIElement			//TODO take the right class from ReMa
	{
	public:
		UIImage(Texture* image, Vector2 position, float size);	//error with Texture copy

		Texture*	m_image;
	};

	class UICanvas
	{
	public:
		UICanvas() = default;
		UICanvas(UICanvas& other) = delete;
		~UICanvas() = default;

		UICanvas& operator=(const UICanvas& other) = delete;

		void	AddElement(unique_ptr<UIElement> element);	//TO USE: uiManager.AddElement(std::make_unique<UIText>("Score: 10", pos, size));

		void	Render(IGUI& gui);

	private:
		vector<unique_ptr<UIElement>>	m_elements;
	};

	class UIManager
	{
	public:
		UIManager() = delete;
		UIManager(void* nativeWindow);
		UIManager(UIManager& other) = delete;
		~UIManager() = default;

		UIManager&	operator=(const UIManager& other) = delete;

		UICanvas&	GetCanvas();

		void		RenderCanvas();

	private:
		UICanvas				m_canvas;
		std::unique_ptr<IGUI>	m_gui;
	};
}

#endif // !HUD
