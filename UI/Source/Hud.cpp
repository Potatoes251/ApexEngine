#include "Hud.h"

using namespace Apex::UserInterface;

//UIElements
UIElement::UIElement(Vector2 position, float size, UIAnchor anchor) :
	m_position(position), m_size(size), m_anchor(anchor) {}

UIText::UIText(string text, Vector2 position, float size, UIAnchor anchor, FontID font) :
	UIElement(position, size, anchor), m_text(text), m_font(font) {}

void UIText::Draw(IGUI& gui)
{
	Vector2 pixelPos = ApplyAnchor(m_position, m_anchor, gui.GetScreenSize());

	gui.DrawText2D(m_text, pixelPos[0], pixelPos[1], m_size, m_font);
}

Vector2 UIElement::ApplyAnchor(Vector2 pos, UIAnchor anchor, Vector2 screenSize)
{
	Vector2 anchorOffset{ 0,0 };

	switch (anchor)
	{
	case UIAnchor::TopLeft:
		anchorOffset = { 0						,0 }; 
		break;
	case UIAnchor::TopCenter:
		anchorOffset = { screenSize[0] * 0.5f	, 0 }; 
		break;
	case UIAnchor::TopRight:
		anchorOffset = { screenSize[0]			, 0 }; 
		break;

	case UIAnchor::MiddleLeft:	
		anchorOffset = { 0						, screenSize[1] * 0.5f }; 
		break;
	case UIAnchor::Center:
		anchorOffset = { screenSize[0] * 0.5f	, screenSize[1] * 0.5f }; 
		break;
	case UIAnchor::MiddleRight:
		anchorOffset = { screenSize[0]			, screenSize[1] * 0.5f }; 
		break;

	case UIAnchor::BottomLeft:
		anchorOffset = { 0						, screenSize[1] }; 
		break;
	case UIAnchor::BottomCenter:
		anchorOffset = { screenSize[0] * 0.5f	, screenSize[1] }; 
		break;
	case UIAnchor::BottomRight:
		anchorOffset = { screenSize[0]			, screenSize[1] }; 
		break;
	}

	return anchorOffset + Vector2(pos[0] * screenSize[0], pos[1] * screenSize[1]);
}

UIImage::UIImage(Texture* image, Vector2 position, float size, UIAnchor anchor) :
	UIElement(position, size, anchor), m_image(image) {}

//Canvas
void UICanvas::AddElement(unique_ptr<UIElement> element)
{
	m_elements.push_back(std::move(element));
}

void UICanvas::Render(IGUI& gui)
{
	for (auto& element : m_elements) element->Draw(gui);
}


//UIManager
UIManager::UIManager(void* nativeWindow)
{
	m_gui = unique_ptr<IGUI>(CreateGUI(nativeWindow));

	// mess with the rest of rendering
	//glDisable(GL_DEPTH_TEST);	//TOO SEE IF IYT AFFECT 3D RENDERING
}

UICanvas& Apex::UserInterface::UIManager::GetCanvas()
{
	return m_canvas;
}

void UIManager::RenderCanvas()
{
	m_gui.get()->BeginFrame();
	glDisable(GL_DEPTH_TEST);

	m_canvas.Render(*m_gui);

	m_gui.get()->EndFrame();
	glEnable(GL_DEPTH_TEST);
}
