#include "ComponentsViewer.h"
#include "ResourceHandle.h"
#include "MeshRenderer.h"

#include "MeshCollider.h"
#include "BoxCollider.h"
#include "CapsuleCollider.h"
#include "Rigidbody.h"

#include "ScriptComponent.h"

#include "Lighting/Lights.h"

#include "FirstPersonCamera.h"
#include "ThirdPersonCamera.h"

#include "CharacterController.h"

using namespace Apex::Editor;
using namespace Apex::UserInterface;
using namespace Apex::Rendering;
using namespace Apex::Resources;
using namespace Apex::Physic;
using namespace Apex::Scripting;
using namespace Apex::Lighting;

static constexpr float LABEL_WIDTH = 200.f;
static constexpr float ROW_PAD = 5.f;

// Accent colors per component category
static constexpr uint32_t COL_RENDER = 0xFF81C784; // green
static constexpr uint32_t COL_PHYSICS = 0xFFFF69B4; // pink
static constexpr uint32_t COL_SCRIPT = 0xFFFFA500; // orange
static constexpr uint32_t COL_LIGHT = 0xFFFFEE58; // yellow
static constexpr uint32_t COL_CAMERA = 0xFF80DEEA; // cyan
static constexpr uint32_t COL_DEFAULT = 0xFF90A4AE; // grey

ComponentsViewer::ComponentsViewer(IGUI& gui, Scene& scene, 
	Apex::Resources::ResourceManager& resourceManager, 
	Apex::Physic::PhysicSystem& phys, LuaManager& lua)
	: m_gui(gui), m_scene(scene), m_resourceManager(resourceManager), 
	m_meshPicker(gui, m_resourceManager), m_texturePicker(gui, m_resourceManager), 
	m_phys(phys), m_luaManager(lua), m_scriptPicker(gui)
{
}

static uint32_t AccentForType(const char* typeName)
{
	if (!typeName) return COL_DEFAULT;
	std::string type(typeName);
	if (type.find("MeshRenderer") != std::string::npos) return COL_RENDER;
	if (type.find("Light") != std::string::npos) return COL_LIGHT;
	if (type.find("Rigidbody") != std::string::npos ||
		type.find("Collider") != std::string::npos) return COL_PHYSICS;
	if (type.find("Script") != std::string::npos) return COL_SCRIPT;
	if (type.find("Camera") != std::string::npos) return COL_CAMERA;
	if (type.find("Character") != std::string::npos) return COL_PHYSICS;
	return COL_DEFAULT;
}

void Apex::Editor::ComponentsViewer::ClearSelection()
{
	m_selectedObj = nullptr; 
	m_selectedComp = nullptr; 
}

void ComponentsViewer::Draw()
{
	m_changed = false;

	m_gui.PushFont(FontID::Default);
	m_gui.BeginPanel("Components");

	if (!m_selectedObj)
	{
		m_gui.TextDisabled("No object selected.");
		m_gui.EndPanel();
		m_gui.PopFont();
		return;
	}

	m_gui.PushFont(FontID::Large);
	m_gui.Text(m_selectedObj->GetName());
	m_gui.PopFont();
	m_gui.Separator();

	m_gui.PushStyleVariable(StyleVariable::ItemSpacing, 4.f, ROW_PAD);

	DrawTransform(m_selectedObj);

	for (Component* comp : m_selectedObj->GetComponents())
	{
		if (!comp) continue;
		DrawComponent(comp);
	}

	if (m_pendingDelete)
	{
		m_selectedObj->RemoveComponent(m_pendingDelete, m_phys);
		m_pendingDelete = nullptr;
	}

	m_gui.PopStyleVariable();

	m_gui.Separator();

	float availableWidth = m_gui.GetAvailableSize()[0];
	m_gui.SetNextItemWidth(availableWidth);
	if (m_gui.ButtonSized("+ Add Component", { availableWidth, 28.f }))
		m_gui.OpenPopup("AddComponent");

	if (m_gui.BeginPopup("AddComponent") && m_selectedObj)
	{
		if (m_selectedObj->GetComponent<MeshRenderer>() == nullptr &&
			m_gui.MenuItem("Mesh Renderer"))
		{
			auto& comp = m_selectedObj->AddComponent<MeshRenderer>(m_resourceManager);
			comp.SetEnabled(true);
		}

		if (m_selectedObj->GetComponent<RigidBodyComponent>() == nullptr && m_gui.MenuItem("Rigidbody"))
			m_selectedObj->AddComponent<RigidBodyComponent>(1.0f);

		RigidBodyComponent* rb = m_selectedObj->GetComponent<RigidBodyComponent>();
		if (rb && m_selectedObj->GetComponent<Collider>() == nullptr)
		{
			auto* mr = m_selectedObj->GetComponent<MeshRenderer>();
			if (mr && m_gui.MenuItem("Mesh Collider"))
			{
				m_selectedObj->AddComponent<MeshCollider>(
					m_resourceManager.Get<Mesh>(mr->GetMesh()->GetPath()),
					MeshColliderType::Convex, false);
				m_phys.CreateActor(*m_selectedObj->GetComponentsByType<Collider>().back(), *rb);
			}
			else if (m_gui.MenuItem("Box Collider"))
			{
				m_selectedObj->AddComponent<BoxCollider>(LibMath::Vector3(1.f, 1.f, 1.f), false);
				m_phys.CreateActor(*m_selectedObj->GetComponentsByType<Collider>().back(), *rb);
			}
			else if (m_gui.MenuItem("Capsule Collider"))
			{
				m_selectedObj->AddComponent<CapsuleCollider>(0.5f, 0.2f, false);
				m_phys.CreateActor(*m_selectedObj->GetComponentsByType<Collider>().back(), *rb);
			}
		}

		if (m_gui.MenuItem("Lua Script"))
			m_selectedObj->AddComponent<ScriptComponent>(m_luaManager);

		m_gui.Separator();
		if (m_gui.MenuItem("Directional Light"))
			m_selectedObj->AddComponent<DirectionalLightComponent>();
		if (m_gui.MenuItem("Point Light"))
			m_selectedObj->AddComponent<PointLightComponent>();
		if (m_gui.MenuItem("Spot Light"))
			m_selectedObj->AddComponent<SpotLightComponent>();

		m_gui.Separator();
		if (m_gui.MenuItem("First Person Camera"))
			m_selectedObj->AddComponent<FirstPersonCamera>();
		if (m_gui.MenuItem("Third Person Camera"))
			m_selectedObj->AddComponent<ThirdPersonCamera>();

		m_gui.Separator();
		if (m_gui.MenuItem("Character Controller"))
			m_selectedObj->AddComponent<Controller::CharacterController>();

		m_gui.EndPopup();
	}

	m_gui.EndPanel();

	m_meshPicker.Draw();
	m_texturePicker.Draw();
	m_scriptPicker.Draw();

	m_gui.PopFont();
}

void ComponentsViewer::DrawTransform(Object* obj)
{
	bool open = m_gui.CollapsingHeader("Transform", true);

	if (!open) return;

	LibMath::Transform transform = obj->GetLocalTransform();

	bool externalChange = (obj != m_lastTransformObj);
	if (externalChange)
	{
		m_cachedEuler = transform.getEulerRotation() * (180.0f / g_Pi);
		m_lastTransformObj = obj;
	}

	LibMath::Vector3 pos = transform.getPosition();
	LibMath::Vector3 scale = transform.getScale();

	bool changed = false;
	changed |= DrawVec3("Position", pos, -1000.f, 1000.f);
	changed |= DrawVec3("Rotation", m_cachedEuler, 0.f, 360.f);
	changed |= DrawVec3("Scale", scale, -1000.f, 1000.f);

	if (changed)
	{
		transform.setPosition(pos);
		transform.setEulerRotation(m_cachedEuler * (g_Pi / 180.f));
		transform.setScale(scale);
		obj->SetLocalTransform(transform);
	}
}

bool ComponentsViewer::DrawVec3(const std::string& name, LibMath::Vector3& v, float min, float max)
{
	bool changed = false;

	m_gui.AlignTextToFramePadding();
	m_gui.Text(name);
	m_gui.SameLine(LABEL_WIDTH);

	m_gui.PushID(name.c_str());

	struct Channel 
	{ 
		const char* id; 
		float* ptr; 
		uint32_t col; 
	};
	Channel channels[3] = 
	{ 
		{"X", &v[0], 0xFFFF0000},
		{"Y", &v[1], 0xFF00FF00}, 
		{"Z", &v[2], 0xFF0000FF}
	};

	float available = m_gui.GetAvailableSize()[0];
	float sliderWidth = (available - 36.f) / 3.f;

	for (int i = 0; i < 3; ++i)
	{
		if (i > 0) m_gui.SameLine(0.f, 2.f);

		m_gui.PushColor(StyleColor::Text, Color{
			((channels[i].col >> 16) & 0xFF) / 255.f,
			((channels[i].col >> 8) & 0xFF) / 255.f,
			((channels[i].col >> 0) & 0xFF) / 255.f, 1.f });
		m_gui.Text(channels[i].id);
		m_gui.PopColor();

		m_gui.SameLine(0.f, 2.f);
		m_gui.SetNextItemWidth(sliderWidth);
		changed |= m_gui.InputFloat(std::string("##") + channels[i].id + std::to_string(i), channels[i].ptr);
	}

	m_gui.PopID();
	return changed;
}

void Apex::Editor::ComponentsViewer::DrawComponent(Apex::Component* comp)
{
	uint32_t accent = AccentForType(comp->GetTypeName());

	float r = ((accent >> 16) & 0xFF) / 255.f;
	float g = ((accent >> 8) & 0xFF) / 255.f;
	float b = ((accent >> 0) & 0xFF) / 255.f;

	m_gui.PushColor(StyleColor::Header, { r * 0.6f, g * 0.6f, b * 0.6f, 1.f });
	m_gui.PushColor(StyleColor::HeaderHovered, { r , g , b , 1.f });

	std::string label = std::string(comp->GetTypeName()) + "##comp" + std::to_string(comp->GetId());
	bool open = m_gui.CollapsingHeader(label, false);

	m_gui.PopColor(2);

	HandleSelection(comp);

	if (m_gui.BeginContextMenu(std::to_string(comp->GetId()).c_str()))
	{
		if (m_gui.MenuItem("Delete"))
			m_pendingDelete = comp;
		m_gui.EndPopup();
	}

	if (!open) return;

	m_gui.PushStyleVariable(StyleVariable::IndentSpacing, 8.f);

	if (auto* script = dynamic_cast<ScriptComponent*>(comp))
	{
		DrawScriptComponent(script);
	}
	else
	{
		for (auto& var : comp->GetExposedVariables())
			DrawVar(var);
	}

	if (m_changed) m_selectedObj->OnChanged();
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

	m_gui.PopStyleVariable();
}

void Apex::Editor::ComponentsViewer::HandleSelection(Apex::Component* comp)
{
	if (m_gui.IsItemClicked())
		m_selectedComp = comp;
}

void Apex::Editor::ComponentsViewer::DrawScriptComponent(Apex::Scripting::ScriptComponent* script)
{
	m_gui.AlignTextToFramePadding();
	m_gui.Text("Script:");
	m_gui.SameLine(LABEL_WIDTH);
	std::string name = script->GetScriptPath().empty() ? "None" : std::filesystem::path(script->GetScriptPath()).filename().string();
	m_gui.TextDisabled(name);

	float width = m_gui.GetAvailableSize()[0];
	if (m_gui.ButtonSized("Select Script##" + std::to_string(script->GetId()), { width, 22.f }))
		m_scriptPicker.Open(script);

	// Exposed variables from Lua
	for (auto& var : script->GetExposedVariables())
	{
		switch (var.type)
		{
		case ExposedVar::Float:
		{
			m_gui.AlignTextToFramePadding();
			m_gui.Text(var.name);
			m_gui.SameLine(LABEL_WIDTH);
			m_gui.SetNextItemWidth(-1.f);
			float value = std::get<float>(var.value);
			if (m_gui.InputFloat(std::string("##") + var.name, &value))
			{
				var.value = value; 
				script->SetVariable(var.name, var);
			}
			break;
		}
		case ExposedVar::Bool:
		{
			m_gui.AlignTextToFramePadding();
			m_gui.Text(var.name);
			m_gui.SameLine(LABEL_WIDTH);
			bool value = std::get<bool>(var.value);
			if (m_gui.Checkbox(std::string("##") + var.name, &value))
			{
				var.value = value; 
				script->SetVariable(var.name, var);
			}
			break;
		}
		case ExposedVar::String:
		{
			m_gui.AlignTextToFramePadding();
			m_gui.Text(var.name);
			m_gui.SameLine(LABEL_WIDTH);
			m_gui.SetNextItemWidth(-1.f);
			char buf[256];
			const auto& str = std::get<std::string>(var.value);
			strncpy(buf, str.c_str(), 255); buf[255] = '\0';
			if (m_gui.InputText(std::string("##") + var.name, buf, 256))
			{
				var.value = std::string(buf); 
				script->SetVariable(var.name, var);
			}
			break;
		}
		default: break;
		}
	}
}

void Apex::Editor::ComponentsViewer::DrawVar(ExposedVar& var)
{
	std::string wid = std::string("##") + var.name;
	switch (var.type)
	{
	case ExposedVar::Float:
		m_gui.AlignTextToFramePadding();
		m_gui.Text(var.name);
		m_gui.SameLine(LABEL_WIDTH);
		m_gui.SetNextItemWidth(-1.f);
		if (m_gui.InputFloat(wid, (float*)var.data))
			m_changed = true;
		break;

	case ExposedVar::Int:
		m_gui.AlignTextToFramePadding();
		m_gui.Text(var.name);
		m_gui.SameLine(LABEL_WIDTH);
		m_gui.SetNextItemWidth(-1.f);
		if (m_gui.InputInt(wid, (int*)var.data))
			m_changed = true;
		break;

	case ExposedVar::Bool:
		m_gui.AlignTextToFramePadding();
		m_gui.Text(var.name);
		m_gui.SameLine(LABEL_WIDTH);
		if (m_gui.Checkbox(wid, (bool*)var.data))
			m_changed = true;
		break;

	case ExposedVar::String:
		m_gui.AlignTextToFramePadding();
		m_gui.Text(var.name);
		m_gui.SameLine(LABEL_WIDTH);
		m_gui.SetNextItemWidth(-1.f);
		if (m_gui.InputText(wid, (char*)var.data, 256))
			m_changed = true;
		break;

	case ExposedVar::Enum:
		DrawComponentEnum(var);
		break;

	case ExposedVar::Vector3:
		DrawComponentVector3(var);
		break;

	case ExposedVar::PhysMaterial:
		DrawComponentPhysMaterial(var);
		break;

	case ExposedVar::Mesh:
		DrawComponentMesh(var);
		break;

	case ExposedVar::Texture:
		DrawComponentTexture(var);
		break;

	case ExposedVar::Script:
		DrawComponentScript(var);
		break;
	}
}

void Apex::Editor::ComponentsViewer::DrawComponentEnum(ExposedVar var)
{
	m_gui.AlignTextToFramePadding();
	m_gui.Text(var.name);
	m_gui.SameLine(LABEL_WIDTH);
	m_gui.SetNextItemWidth(-1.f);

	int value = *(int*)var.data;
	std::string items;
	for (const auto& n : var.enumNames) items += n + '\0';
	if (m_gui.Combo(std::string("##") + var.name, &value, items.c_str()))
	{
		*(int*)var.data = value; 
		m_changed = true;
	}
}

void Apex::Editor::ComponentsViewer::DrawComponentVector3(ExposedVar var)
{
	float* v = (float*)var.data;
	LibMath::Vector3 vec(v[0], v[1], v[2]);
	if (DrawVec3(var.name, vec, -1000.f, 1000.f))
	{
		v[0] = vec[0]; v[1] = vec[1]; v[2] = vec[2]; 
		m_changed = true;
	}
}

void Apex::Editor::ComponentsViewer::DrawComponentPhysMaterial(ExposedVar var)
{
	PhysicMaterial* mat = (PhysicMaterial*)var.data;

	auto row = [&](const char* label, const char* id, float& val, float mn, float mx)
		{
			m_gui.AlignTextToFramePadding();
			m_gui.Text(label);
			m_gui.SameLine(LABEL_WIDTH);
			m_gui.SetNextItemWidth(-1.f);
			if (m_gui.InputFloat(id, &val))
				m_changed = true;
		};

	row("Static Friction", "##sf", mat->GetStaticFriction(), 0.f, 1.f);
	row("Dynamic Friction", "##df", mat->GetDynamicFriction(), 0.f, 1.f);
	row("Bounciness", "##bo", mat->GetBounciness(), 0.f, 1.f);
}

void Apex::Editor::ComponentsViewer::DrawComponentMesh(ExposedVar var)
{
	auto* mesh = (ResourceHandle<Mesh>*)var.data;

	m_gui.AlignTextToFramePadding();
	m_gui.Text("Mesh:");
	m_gui.SameLine(LABEL_WIDTH);
	m_gui.TextDisabled(mesh->IsValid() ? (*mesh)->GetPath() : "None");

	float width = m_gui.GetAvailableSize()[0];
	if (m_gui.ButtonSized(std::string("Select##") + var.name, { width, 22.f }))
		m_meshPicker.Open(mesh);
}

void Apex::Editor::ComponentsViewer::DrawComponentTexture(ExposedVar var)
{
	auto* texture = (ResourceHandle<Texture>*)var.data;

	m_gui.AlignTextToFramePadding();
	m_gui.Text("Texture:");
	m_gui.SameLine(LABEL_WIDTH);
	m_gui.TextDisabled(texture->IsValid() ? (*texture)->GetPath() : "None");

	float w = m_gui.GetAvailableSize()[0];
	if (m_gui.ButtonSized(std::string("Select##") + var.name, { w, 22.f }))
		m_texturePicker.Open(texture);
}

void ComponentsViewer::DrawComponentScript(ExposedVar var)
{
	auto* script = (ScriptComponent*)var.data;

	m_gui.AlignTextToFramePadding();
	m_gui.Text("Script:");
	m_gui.SameLine(LABEL_WIDTH);
	m_gui.TextDisabled(script->GetScriptPath().empty() ? "None" : script->GetScriptPath());

	float w = m_gui.GetAvailableSize()[0];
	if (m_gui.ButtonSized("Select Script##" + std::to_string(script->GetId()), { w, 22.f }))
		m_scriptPicker.Open(script);
}