#include "ComponentsViewer.h"

using namespace Apex::Editor;
using namespace Apex::UserInterface;
using namespace Apex::Rendering;

ComponentsViewer::ComponentsViewer(IGUI& gui, Scene& scene)
	: m_gui(gui), m_scene(scene)
{
}

void ComponentsViewer::Draw()
{
	m_gui.PushFont(FontID::Default);
	m_gui.BeginPanel("Components");

	if (m_selectedObj)
	{
		for (Component* comp : m_selectedObj->GetComponents())
		{
			DrawComponent(comp);
		}
	}

	m_gui.EndPanel();
	m_gui.PopFont();
}

void Apex::Editor::ComponentsViewer::DrawComponent(Apex::Component* comp)
{
	bool open = DrawTreeComp(comp);
	
	HandleSelection(comp);

	if (open)
	{
		comp->DrawVariables(m_gui);

		m_gui.EndTreeNode();
	}
}

bool Apex::Editor::ComponentsViewer::DrawTreeComp(Apex::Component* comp)
{
	TreeNodeFlags flags = TreeNodeFlags::SpanAvailWidth;
	if (comp == m_selectedComp) flags = flags | TreeNodeFlags::Selected;

	std::string label = std::to_string(comp->GetId()) + ". " + comp->GetTypeName();
	return m_gui.BeginTreeNode(label.c_str(), flags);
}

void Apex::Editor::ComponentsViewer::HandleSelection(Apex::Component* comp)
{
	if (m_gui.IsItemClicked())
		m_selectedComp = comp;
}
