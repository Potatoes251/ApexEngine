#ifndef HUD
#define HUD

#include "UI.h"
#include <string>
#include <vector>
#include <memory>
#include "LibMath/Vector/Vector2.h"
#include "Texture.h"
#include "ResourceHandle.h"

using std::string;
using std::vector;
using std::unique_ptr;
using LibMath::Vector2;
using Apex::UserInterface::FontID;
using Apex::Resources::ResourceHandle;

namespace Apex::UserInterface
{
	enum class UIAnchor
	{
		TopLeft,
		TopCenter,
		TopRight,

		MiddleLeft,
		Center,
		MiddleRight,

		BottomLeft,
		BottomCenter,
		BottomRight
	};

	struct UIElement
	{
	public:
				UIElement(Vector2 position, float size, UIAnchor anchor);
		virtual ~UIElement() = default;

		virtual	void	Draw(IGUI& gui) = 0;

				Vector2 ApplyAnchor(Vector2 pos, UIAnchor anchor, Vector2 screenSize);

		Vector2		m_position;		//in percentage -> [0 , 1]
		float		m_size;
		UIAnchor	m_anchor;
	};

	struct UIText : UIElement
	{
	public:
		UIText(const string& text, Vector2 position, float size = 1.f, UIAnchor anchor = UIAnchor::Center, FontID font = FontID::Default);

		void	Draw(IGUI& gui) override;

		string	m_text;
		FontID	m_font;
	};
	
	struct UIImage : UIElement
	{
	public:
		UIImage(const ResourceHandle<Texture>& image, Vector2 position, float size = 1.f, Vector2 imageSize = Vector2(1.f), UIAnchor anchor = UIAnchor::Center);

		void	Draw(IGUI& gui) override;

		ResourceHandle<Texture>	m_image;
		Vector2					m_imageSize;
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
		UIManager(IGUI* gui);
		UIManager(UIManager& other) = delete;
		~UIManager() = default;

		UIManager&	operator=(const UIManager& other) = delete;

		UICanvas&	GetCanvas();

		void		RenderCanvas();

	private:
		UICanvas	m_canvas;
		IGUI*		m_gui;
	};
}

#endif // !HUD
