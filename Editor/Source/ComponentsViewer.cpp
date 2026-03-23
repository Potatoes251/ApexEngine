#include "ComponentsViewer.h"
#include "ResourceHandle.h"

using namespace Apex::Editor;
using namespace Apex::UserInterface;
using namespace Apex::Rendering;
using namespace Apex::Resources;

ComponentsViewer::ComponentsViewer(IGUI& gui, Scene& scene, Apex::Resources::ResourceManager& resourceManager)
	: m_gui(gui), m_scene(scene), m_resourceManager(resourceManager), m_meshPicker(gui, m_resourceManager), m_texturePicker(gui, m_resourceManager)
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

	m_meshPicker.Draw();
	m_texturePicker.Draw();

	m_gui.PopFont();
}

void Apex::Editor::ComponentsViewer::DrawComponent(Apex::Component* comp)
{
	bool open = DrawTreeComp(comp);
	
	HandleSelection(comp);

	if (!open)
		return;

	for (auto& var : comp->GetExposedVariables())
	{
		switch (var.type)
		{
			case ExposedVar::Float:
				m_gui.SliderFloat(var.name, (float*)var.data, 0.0f, 10.0f);
				break;

			case ExposedVar::Int:
				m_gui.SliderInt(var.name, (int*)var.data, 0.0f, 10.0f);
				break;

			case ExposedVar::Bool:
				m_gui.Checkbox(var.name, (bool*)var.data);
				break;

			case ExposedVar::String:
				m_gui.InputText(var.name, (char*)var.data, 256);
				break;

			case ExposedVar::Enum:
			{
				DrawComponentEnum(var);
				break;
			}

			case ExposedVar::Vector3:
			{
				DrawComponentVector3(var);
				break;
			}

			case ExposedVar::PhysMaterial:
			{
				DrawComponentPhysMaterial(var);
				break;
			}

			case ExposedVar::Mesh:
			{
				DrawComponentMesh(var);
				break;
			}

			case ExposedVar::Texture:
			{
				DrawComponentTexture(var);
				break;
			}
		}
	}

	m_gui.EndTreeNode();
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

void Apex::Editor::ComponentsViewer::DrawComponentEnum(ExposedVar var)
{
	int value = *(int*)var.data;

	std::string items;
	for (const auto& name : var.enumNames)
		items += name + '\0';

	if (m_gui.Combo(var.name, &value, items.c_str()))
		*(int*)var.data = value;
}

void Apex::Editor::ComponentsViewer::DrawComponentVector3(ExposedVar var)
{
	float* v = (float*)var.data;

	m_gui.Text(var.name);

	m_gui.SliderFloat("X", &v[0], -1000.0f, 1000.0f);
	m_gui.SliderFloat("Y", &v[1], -1000.0f, 1000.0f);
	m_gui.SliderFloat("Z", &v[2], -1000.0f, 1000.0f);
}

void Apex::Editor::ComponentsViewer::DrawComponentPhysMaterial(ExposedVar var)
{
	PhysicMaterial* mat = (PhysicMaterial*)var.data;

	m_gui.SliderFloat("Static friction", &mat->GetStaticFriction(), 0.0f, 1.0f);
	m_gui.SliderFloat("Dynamic friction", &mat->GetDynamicFriction(), 0.0f, 1.0f);
	m_gui.SliderFloat("Bounciness", &mat->GetBounciness(), 0.0f, 1.0f);
}

void Apex::Editor::ComponentsViewer::DrawComponentMesh(ExposedVar var)
{
	auto* mesh = (ResourceHandle<Mesh>*)var.data;

	if (mesh->IsValid())
		m_gui.Text("Mesh: " + (*mesh)->GetPath());
	else
		m_gui.Text("Mesh: None");

	if (m_gui.Button(("Select##" + var.name).c_str()))
		m_meshPicker.Open(mesh);
}

void Apex::Editor::ComponentsViewer::DrawComponentTexture(ExposedVar var)
{
	auto* texture = (ResourceHandle<Texture>*)var.data;

	if (texture->IsValid())
		m_gui.Text("Texture: " + (*texture)->GetPath());
	else
		m_gui.Text("Texture: None");

	if (m_gui.Button(("Select##" + var.name).c_str()))
		m_texturePicker.Open(texture);
}
