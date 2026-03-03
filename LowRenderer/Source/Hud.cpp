#include "Hud.h"

using namespace Apex::UserInterface;

UIElement::UIElement(Vector2 position, Vector2 size) :
	m_position(position), m_size(size) {}

UIText::UIText(string text, Vector2 position, Vector2 size) :
	UIElement(position, size), m_text(text) {}

void UIText::Draw()
{

}

UIImage::UIImage(Texture* image, Vector2 position, Vector2 size) :
	UIElement(position, size), m_image(image) {}

UIManager::UIManager()
{
	glDisable(GL_DEPTH_TEST);	//TOO SEE IF IYT AFFECT 3D RENDERING
}

void UIManager::AddElement(unique_ptr<UIElement> element)
{
	m_elements.push_back(std::move(element));
}

void UIManager::Render()
{
	for (auto& element : m_elements) element->Draw();
}