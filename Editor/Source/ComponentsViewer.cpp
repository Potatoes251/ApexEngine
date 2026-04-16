#include "ComponentsViewer.h"
#include "ResourceHandle.h"
#include "MeshRenderer.h"
#include "MeshCollider.h"
#include "BoxCollider.h"
#include "CapsuleCollider.h"
#include "Rigidbody.h"
#include "ScriptComponent.h"
#include "Lighting/Lights.h"
#include "Animator.h"

using namespace Apex::Editor;
using namespace Apex::UserInterface;
using namespace Apex::Rendering;
using namespace Apex::Resources;
using namespace Apex::Physic;
using namespace Apex::Scripting;
using namespace Apex::Lighting;

ComponentsViewer::ComponentsViewer(IGUI& gui, Scene& scene, 
	Apex::Resources::ResourceManager& resourceManager, 
	Apex::Physic::PhysicSystem& phys, LuaManager& lua)
	: m_gui(gui), m_scene(scene), m_resourceManager(resourceManager), 
	m_meshPicker(gui, m_resourceManager), m_texturePicker(gui, m_resourceManager), 
	m_phys(phys), m_luaManager(lua), m_scriptPicker(gui)
{
}

void ComponentsViewer::Draw()
{
	m_gui.PushFont(FontID::Default);
	m_gui.BeginPanel("Components");

	if (m_selectedObj && m_gui.Button(" + Add "))
		m_gui.OpenPopup("AddComponent");

	if (m_gui.BeginPopup("AddComponent") && m_selectedObj)
	{
		if (m_selectedObj->GetComponent<MeshRenderer>() == nullptr && 
			m_gui.MenuItem("Mesh"))
		{
			m_selectedObj->AddComponent<MeshRenderer>(m_resourceManager);
		}

		if (m_selectedObj->GetComponent<RigidBodyComponent>() == nullptr &&
			m_gui.MenuItem("Rigidbody"))
		{
			m_selectedObj->AddComponent<RigidBodyComponent>(1.0f);
		}

		RigidBodyComponent* rb = m_selectedObj->GetComponent<RigidBodyComponent>();

		if (rb && m_selectedObj->GetComponent<Collider>() == nullptr)
		{
			auto* meshRenderer = m_selectedObj->GetComponent<MeshRenderer>();

			if (meshRenderer && m_gui.MenuItem("Mesh Collider"))
			{
				m_selectedObj->AddComponent<MeshCollider>(m_resourceManager.Get<Mesh>(
					m_selectedObj->GetComponent<MeshRenderer>()->GetMesh()->GetPath()),
					MeshColliderType::Convex, false);

				m_phys.CreateActor(*m_selectedObj->GetComponentsByType<Collider>().back(), *rb);
			}

			else if (m_gui.MenuItem("Box Collider"))
			{
				m_selectedObj->AddComponent<BoxCollider>(LibMath::Vector3(1.0f, 1.0f, 1.0f), false);

				m_phys.CreateActor(*m_selectedObj->GetComponentsByType<Collider>().back(), *rb);
			}

			else if (m_gui.MenuItem("Capsule Collider"))
			{
				m_selectedObj->AddComponent<CapsuleCollider>(0.5f, 0.2f, false);

				m_phys.CreateActor(*m_selectedObj->GetComponentsByType<Collider>().back(), *rb);
			}
		}

		if (m_gui.MenuItem("Lua Script"))
		{
			m_selectedObj->AddComponent<ScriptComponent>(m_luaManager);
		}

		if (m_gui.MenuItem("Directional Light"))
		{
			m_selectedObj->AddComponent<DirectionalLightComponent>();
		}

		if (m_gui.MenuItem("Point Light"))
		{
			m_selectedObj->AddComponent<PointLightComponent>();
		}

		if (m_gui.MenuItem("Spot Light"))
		{
			m_selectedObj->AddComponent<SpotLightComponent>();
		}

		if (m_gui.MenuItem("Animator"))
		{
			m_selectedObj->AddComponent<Animator>(m_resourceManager, m_selectedObj->GetComponent<MeshRenderer>()->GetMesh());
		}

		m_gui.EndPopup();
	}

	if (m_selectedObj)
	{
		for (Component* comp : m_selectedObj->GetComponents())
		{
			if (!comp)
				continue;

			DrawComponent(comp);
		}

		if (m_pendingDelete)
		{
			m_selectedObj->RemoveComponent(m_pendingDelete, m_phys);
			m_pendingDelete = nullptr;
		}
	}

	m_gui.EndPanel();

	m_meshPicker.Draw();
	m_texturePicker.Draw();
	m_scriptPicker.Draw();

	m_gui.PopFont();
}

void Apex::Editor::ComponentsViewer::DrawComponent(Apex::Component* comp)
{
	bool open = DrawTreeComp(comp);
	
	HandleSelection(comp);

	if (m_gui.BeginContextMenu(std::to_string(comp->GetId()).c_str()))
	{
		if (m_gui.MenuItem("Delete"))
		{
			m_pendingDelete = comp;
		}

		m_gui.EndPopup();
	}

	if (!open)
		return;

	for (auto& var : comp->GetExposedVariables())
	{
		switch (var.type)
		{
			case ExposedVar::Float:
				if (m_gui.SliderFloatFormat(var.name, (float*)var.data, 0.0f, 10.0f, 50.f))
					m_changed = true;
				break;

			case ExposedVar::Int:
				if (m_gui.SliderIntFormat(var.name, (int*)var.data, 0, 10, 50.f))
					m_changed = true;
				break;

			case ExposedVar::Bool:
				if (m_gui.CheckboxFormat(var.name, (bool*)var.data))
					m_changed = true;
				break;

			case ExposedVar::String:
				if (m_gui.InputText(var.name, (char*)var.data, 256))
					m_changed = true;
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

			case ExposedVar::Script:
				DrawComponentScript(var);
				break;
		}
	}

	if (m_changed)
		m_selectedObj->OnChanged();

	if (m_meshPicker.HasNewAsset())
	{
		m_selectedObj->OnChanged();
		m_meshPicker.ResetNewAsset();
	}

	if (m_texturePicker.HasNewAsset())
	{
		m_selectedObj->OnChanged();
		m_texturePicker.ResetNewAsset();
	}

	if (m_scriptPicker.HasNewScript())
	{
		m_selectedObj->OnChanged();
		m_scriptPicker.ResetNewScript();
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

	if (m_gui.ComboFormat(var.name, &value, items.c_str()))
	{
		*(int*)var.data = value;
		m_changed = true;
	}
}

void Apex::Editor::ComponentsViewer::DrawComponentVector3(ExposedVar var)
{
	float* v = (float*)var.data;

	m_gui.Text(var.name);

	m_gui.SameLine(150);
	m_changed |= m_gui.SliderFloatFormat("X", &v[0], -1000.0f, 1000.0f, 50.0f);
	m_gui.SameLine();
	m_changed |= m_gui.SliderFloatFormat("Y", &v[1], -1000.0f, 1000.0f, 50.0f);
	m_gui.SameLine();
	m_changed |= m_gui.SliderFloatFormat("Z", &v[2], -1000.0f, 1000.0f, 50.0f);
}

void Apex::Editor::ComponentsViewer::DrawComponentPhysMaterial(ExposedVar var)
{
	PhysicMaterial* mat = (PhysicMaterial*)var.data;

	m_changed |= m_gui.SliderFloatFormat("Static friction", &mat->GetStaticFriction(), 0.0f, 1.0f, 50.0f, 200.f);
	m_changed |= m_gui.SliderFloatFormat("Dynamic friction", &mat->GetDynamicFriction(), 0.0f, 1.0f, 50.0f, 200.f);
	m_changed |= m_gui.SliderFloatFormat("Bounciness", &mat->GetBounciness(), 0.0f, 1.0f, 50.0f, 200.f);
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

void ComponentsViewer::DrawComponentScript(ExposedVar var)
{
	auto* script = (ScriptComponent*)var.data;

	m_gui.Text("Script: " + script->GetScriptPath());

	if (m_gui.Button(("Select Script##" + std::to_string(script->GetId())).c_str()))
		m_scriptPicker.Open(script);
}