#ifndef HUD
#define HUD

#include <string>
#include <vector>
#include <memory>
#include "LibMath/Vector/Vector2.h"
#include "Texture.h"

using std::string;
using std::vector;
using std::unique_ptr;
using LibMath::Vector2;

namespace Apex::UserInterface		// can't be 'HUD' as used on top
{
	struct UIElement
	{
	public:
				UIElement(Vector2 position, Vector2 size);
		virtual ~UIElement() = default;

		virtual	void	Draw() = 0;

		Vector2		m_position;
		Vector2		m_size;
	};

	struct UIText : UIElement
	{
	public:
		UIText(string text, Vector2 position, Vector2 size);

		void	Draw() override;

		string	m_text;
	};
	
	struct UIImage : UIElement			//TODO take the right class from ReMa
	{
	public:
		UIImage(Texture* image, Vector2 position, Vector2 size);	//error with Texture copy

		Texture*	m_image;
	};

	class UIManager
	{
	public:
		UIManager();
		UIManager(UIManager& other) = delete;
		~UIManager() = default;

		UIManager& operator=(const UIManager& other) = delete;

		void	AddElement(unique_ptr<UIElement> element);	//TO USE: uiManager.AddElement(std::make_unique<UIText>("Score: 10", pos, size));

		void	Render();

	private:
		vector<unique_ptr<UIElement>>	m_elements;
	};
}

#endif // !HUD
