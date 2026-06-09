#include "ComponentsViewer.h"
#include "ResourceHandle.h"
#include "MeshRenderer.h"

#include "MeshCollider.h"
#include "BoxCollider.h"
#include "CapsuleCollider.h"
#include "Rigidbody.h"
#include "WaterVolume.h"

#include "ScriptComponent.h"
#include "AudioComponent.h"

#include "Lighting/Lights.h"
#include "Animator.h"

#include "FirstPersonCamera.h"
#include "ThirdPersonCamera.h"

#include "CharacterController.h"
#include "AIController.h"

#include "ViewComponent.h"
#include "Waypoint.h"
#include "ProjectileComponent.h"

#include "Command/SetExposedVarCommand.h"
#include "Command/SetScriptVarCommand.h"
#include "Command/RemoveCompCommand.h"
#include "Command/TransformCommand.h"
#include "Command/AddCompCommand.h"
#include "CommandManager.h"

using namespace Apex::Editor;
using namespace Apex::UserInterface;
using namespace Apex::Rendering;
using namespace Apex::Resources;
using namespace Apex::Physic;
using namespace Apex::Scripting;
using namespace Apex::Lighting;
using namespace Apex::Data;
using namespace Apex::CtrlZ;
using namespace Apex::Perception;

// Accent colors per component category
static constexpr uint32_t COL_RENDER = 0xFF81C784; // green
static constexpr uint32_t COL_PHYSICS = 0xFFFF69B4; // pink
static constexpr uint32_t COL_SCRIPT = 0xFFFFA500; // orange
static constexpr uint32_t COL_LIGHT = 0xFFFFEE58; // yellow
static constexpr uint32_t COL_CAMERA = 0xFF80DEEA; // cyan
static constexpr uint32_t COL_AUDIO = 0xFFCE93D8; // purple
static constexpr uint32_t COL_VIEW = 0xFF64B5F6; // blue
static constexpr uint32_t COL_CONTROLLER = 0xFFE57373; // soft red
static constexpr uint32_t COL_PATHFINDING = 0xFF26C6DA;	// teal
static constexpr uint32_t COL_PROJECTILE = 0xFF4DB6AC; // turquoise
static constexpr uint32_t COL_DEFAULT = 0xFF90A4AE; // grey

ComponentsViewer::ComponentsViewer(IGUI* gui, Scene* scene,
	ResourceManager* resourceManager, LuaManager* lua)
	: m_gui(gui), m_scene(scene), m_resourceManager(resourceManager), 
	m_meshPicker(gui, m_resourceManager), m_texturePicker(gui, m_resourceManager), 
	m_materialPicker(gui, m_resourceManager), m_luaManager(lua), m_scriptPicker(gui)
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
	if (type.find("Controller") != std::string::npos) return COL_CONTROLLER;
	if (type.find("WaterVolume") != std::string::npos) return COL_PHYSICS;
	if (type.find("Animator") != std::string::npos) return COL_RENDER;
	if (type.find("Audio") != std::string::npos) return COL_AUDIO;
	if (type.find("View") != std::string::npos) return COL_VIEW;
	if (type.find("Nav") != std::string::npos) return COL_PATHFINDING;
	if (type.find("Projectile") != std::string::npos) return COL_PROJECTILE;
	return COL_DEFAULT;
}

void Apex::Editor::ComponentsViewer::ClearSelection()
{
	m_selectedObj = nullptr; 
	m_selectedComp = nullptr;
	m_pendingDelete = nullptr;
}

void Apex::Editor::ComponentsViewer::SetThumbnailRenderer(ThumbnailRenderer* t)
{
	m_thumbnailRenderer = t;
	m_meshPicker.SetThumbnailRenderer(t);
	m_texturePicker.SetThumbnailRenderer(t);
	m_materialPicker.SetThumbnailRenderer(t);
}

void ComponentsViewer::Draw()
{
	m_changed = false;

	m_gui->PushFont(FontID::Default);
	m_gui->BeginPanel("Components");

	if (!m_selectedObj)
	{
		m_gui->TextDisabled("No object selected.");
		m_gui->EndPanel();
		m_gui->PopFont();
		return;
	}

	DrawHeader();
	DrawComponentsList();

	if (m_pendingDelete)
	{
		CommandManager::Get().Execute(
			std::make_unique<RemoveCompCommand>(m_selectedObj, m_pendingDelete));
		m_pendingDelete = nullptr;
	}

	DrawAddComponentButton();
	DrawAddComponentPopup();

	m_gui->EndPanel();

	m_meshPicker.Draw();
	m_materialPicker.Draw();
	m_texturePicker.Draw();
	m_scriptPicker.Draw();

	m_gui->PopFont();
}

void ComponentsViewer::DrawHeader()
{
	m_gui->PushFont(FontID::Large);
	m_gui->Text(m_selectedObj->GetName());
	m_gui->PopFont();
	m_gui->Separator();
}

void ComponentsViewer::DrawComponentsList()
{
	m_gui->PushStyleVariable(
		StyleVariable::ItemSpacing, 4.f, ROW_PAD);

	DrawTransform(m_selectedObj);

	for (Component* comp : m_selectedObj->GetComponents())
	{
		if (comp)
			DrawComponent(m_selectedObj, comp);
	}

	m_gui->PopStyleVariable();
	m_gui->Separator();
}

void ComponentsViewer::DrawAddComponentButton()
{
	float width = m_gui->GetAvailableSize()[0];

	m_gui->SetNextItemWidth(width);

	if (m_gui->ButtonSized("+ Add Component", { width, 28.f }))
		m_gui->OpenPopup("AddComponent");
}

void ComponentsViewer::DrawAddComponentPopup()
{
	if (!m_gui->BeginPopup("AddComponent") || !m_selectedObj)
		return;

	DrawRenderComponentsMenu();
	DrawPhysicsComponentsMenu();
	DrawScriptComponentsMenu();
	DrawLightComponentsMenu();
	DrawCameraComponentsMenu();
	DrawControllerComponentsMenu();
	DrawMiscComponentsMenu();

	m_gui->EndPopup();
}

void ComponentsViewer::DrawRenderComponentsMenu()
{
	if (m_selectedObj->GetComponent<MeshRenderer>() == nullptr &&
		m_gui->MenuItem("Mesh Renderer"))
	{
		CommandManager::Get().Execute(std::make_unique<AddCompCommand>(m_selectedObj,
			[this](Object* obj)
			{
				Component* comp = &obj->AddComponent<MeshRenderer>(m_resourceManager);
				return comp->GetId();
			}));
	}

	if (m_gui->MenuItem("Animator"))
	{
		CommandManager::Get().Execute(std::make_unique<AddCompCommand>(m_selectedObj,
			[this](Object* obj)
			{
				Component* comp = &obj->AddComponent<Animator>(obj->GetComponent<MeshRenderer>()->GetModel());
				return comp->GetId();
			}));
	}

	m_gui->Separator();
}

void ComponentsViewer::DrawPhysicsComponentsMenu()
{
	if (m_selectedObj->GetComponent<RigidBodyComponent>() == nullptr &&
		m_gui->MenuItem("Rigidbody"))
	{
		CommandManager::Get().Execute(std::make_unique<AddCompCommand>(m_selectedObj,
			[](Object* obj)
			{
				Component* comp = &obj->AddComponent<RigidBodyComponent>(1.0f);
				return comp->GetId();
			}));
	}

	RigidBodyComponent* rb =
		m_selectedObj->GetComponent<RigidBodyComponent>();

	if (rb && m_selectedObj->GetComponent<Collider>() == nullptr)
	{
		auto* mr = m_selectedObj->GetComponent<MeshRenderer>();
		if (mr && m_gui->MenuItem("Mesh Collider"))
		{
			CommandManager::Get().Execute(std::make_unique<AddCompCommand>(m_selectedObj,
				[this](Object* obj)
				{
					Collider& comp = obj->AddComponent<MeshCollider>(
						m_selectedObj->GetComponent<MeshRenderer>()->GetModel(),
						MeshColliderType::Convex, false);

					m_scene->GetPhysic()->CreateActor(comp, *obj->GetComponent<RigidBodyComponent>());
					return comp.GetId();
				}));
		}
		else if (m_gui->MenuItem("Box Collider"))
		{
			CommandManager::Get().Execute(std::make_unique<AddCompCommand>(m_selectedObj,
				[this](Object* obj)
				{
					Collider& comp = obj->AddComponent<BoxCollider>(LibMath::Vector3(1.0f, 1.0f, 1.0f), false);

					m_scene->GetPhysic()->CreateActor(comp, *obj->GetComponent<RigidBodyComponent>());
					return comp.GetId();
				}));
		}
		else if (m_gui->MenuItem("Capsule Collider"))
		{
			CommandManager::Get().Execute(std::make_unique<AddCompCommand>(m_selectedObj,
				[this](Object* obj)
				{
					CapsuleCollider& comp = obj->AddComponent<CapsuleCollider>(0.5f, 0.2f, false);
					m_scene->GetPhysic()->CreateActor(comp, *obj->GetComponent<RigidBodyComponent>());
					return comp.GetId();
				}));
		}
	}

	if (m_gui->MenuItem("WaterVolume"))
	{
		CommandManager::Get().Execute(std::make_unique<AddCompCommand>(m_selectedObj,
			[this](Object* obj)
			{
				WaterVolume& comp = obj->AddComponent<WaterVolume>();
				return comp.GetId();
			}));
	}

	m_gui->Separator();
}

void ComponentsViewer::DrawScriptComponentsMenu()
{
	if (m_gui->MenuItem("Lua Script"))
	{
		CommandManager::Get().Execute(std::make_unique<AddCompCommand>(m_selectedObj,
			[this](Object* obj)
			{
				Component* comp = &obj->AddComponent<ScriptComponent>(m_luaManager);
				return comp->GetId();
			}));
	}

	m_gui->Separator();
}

void ComponentsViewer::DrawLightComponentsMenu()
{
	if (m_gui->MenuItem("Directional Light"))
	{
		CommandManager::Get().Execute(std::make_unique<AddCompCommand>(m_selectedObj,
			[](Object* obj)
			{
				Component* comp = &obj->AddComponent<DirectionalLightComponent>();
				return comp->GetId();
			}));
	}

	if (m_gui->MenuItem("Point Light"))
	{
		CommandManager::Get().Execute(std::make_unique<AddCompCommand>(m_selectedObj,
			[](Object* obj)
			{
				Component* comp = &obj->AddComponent<PointLightComponent>();
				return comp->GetId();

			}));
	}

	if (m_gui->MenuItem("Spot Light"))
	{
		CommandManager::Get().Execute(std::make_unique<AddCompCommand>(m_selectedObj,
			[](Object* obj)
			{
				Component* comp = &obj->AddComponent<SpotLightComponent>();
				return comp->GetId();
			}));
	}

	m_gui->Separator();
}

void ComponentsViewer::DrawCameraComponentsMenu()
{
	if (m_gui->MenuItem("First Person Camera"))
	{
		CommandManager::Get().Execute(std::make_unique<AddCompCommand>(m_selectedObj,
			[](Object* obj)
			{
				Component* comp = &obj->AddComponent<FirstPersonCamera>();
				return comp->GetId();
			}));
	}

	if (m_gui->MenuItem("Third Person Camera"))
	{
		CommandManager::Get().Execute(std::make_unique<AddCompCommand>(m_selectedObj,
			[](Object* obj)
			{
				Component* comp = &obj->AddComponent<ThirdPersonCamera>();
				return comp->GetId();
			}));
	}

	m_gui->Separator();
}

void ComponentsViewer::DrawControllerComponentsMenu()
{
	if (m_gui->MenuItem("Character Controller"))
	{
		CommandManager::Get().Execute(std::make_unique<AddCompCommand>(m_selectedObj,
			[](Object* obj)
			{
				Component* comp = &obj->AddComponent<Controller::CharacterController>();
				return comp->GetId();
			}));
	}

	if (m_gui->MenuItem("AI Controller"))
	{
		CommandManager::Get().Execute(std::make_unique<AddCompCommand>(m_selectedObj,
			[](Object* obj)
			{
				Component* comp = &obj->AddComponent<Controller::AIController>();
				return comp->GetId();
			}));
	}

	m_gui->Separator();
}

void ComponentsViewer::DrawMiscComponentsMenu()
{
	if (m_gui->MenuItem("Audio Component"))
	{
		CommandManager::Get().Execute(std::make_unique<AddCompCommand>(m_selectedObj,
			[this](Object* obj)
			{
				Component* comp = &obj->AddComponent<Audio::AudioComponent>(nullptr);
				return comp->GetId();
			}));
	}

	if (m_gui->MenuItem("View Component"))
	{
		CommandManager::Get().Execute(std::make_unique<AddCompCommand>(m_selectedObj,
			[this](Object* obj)
			{
				Component* comp = &obj->AddComponent<ViewComponent>(m_scene->GetPhysic(), 10.f, 1.5f, 1.f, 10, 10);
				return comp->GetId();
			}));
	}

	if (m_gui->MenuItem("NavGenVolume"))
	{
		CommandManager::Get().Execute(std::make_unique<AddCompCommand>(m_selectedObj,
			[this](Object* obj)
			{
				Component* comp = &obj->AddComponent<Pathfinding::NavGenVolume>();
				return comp->GetId();
			}));
	}

	if (m_gui->MenuItem("ProjectileComponent"))
	{
		CommandManager::Get().Execute(std::make_unique<AddCompCommand>(m_selectedObj,
			[this](Object* obj)
			{
				Component* comp = &obj->AddComponent<Gameplay::ProjectileComponent>();
				return comp->GetId();
			}));
	}
}

void ComponentsViewer::DrawTransform(Object* obj)
{
	bool open = m_gui->CollapsingHeader("Transform", true);

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
		CommandManager::Get().Execute(
			std::make_unique<TransformCommand>(obj, transform, false));
	}
}

bool ComponentsViewer::DrawVec3(const std::string& name, LibMath::Vector3& v, float min, float max)
{
	bool changed = false;

	m_gui->AlignTextToFramePadding();
	m_gui->Text(name);
	m_gui->SameLine(LABEL_WIDTH);

	m_gui->PushID(name.c_str());

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

	float available = m_gui->GetAvailableSize()[0];
	float sliderWidth = (available - 36.f) / 3.f;

	for (int i = 0; i < 3; ++i)
	{
		if (i > 0) m_gui->SameLine(0.f, 2.f);

		m_gui->PushColor(StyleColor::Text, Color{
			((channels[i].col >> 16) & 0xFF) / 255.f,
			((channels[i].col >> 8) & 0xFF) / 255.f,
			((channels[i].col >> 0) & 0xFF) / 255.f, 1.f });
		m_gui->Text(channels[i].id);
		m_gui->PopColor();

		m_gui->SameLine(0.f, 2.f);
		m_gui->SetNextItemWidth(sliderWidth);
		changed |= m_gui->InputFloat(std::string("##") + channels[i].id + std::to_string(i), channels[i].ptr);
	}

	m_gui->PopID();
	return changed;
}

void ComponentsViewer::DrawComponent(Object* obj, Apex::Component* comp)
{
	uint32_t accent = AccentForType(comp->GetTypeName());

	float r = ((accent >> 16) & 0xFF) / 255.f;
	float g = ((accent >> 8) & 0xFF) / 255.f;
	float b = ((accent >> 0) & 0xFF) / 255.f;

	m_gui->PushColor(StyleColor::Header, { r * 0.6f, g * 0.6f, b * 0.6f, 1.f });
	m_gui->PushColor(StyleColor::HeaderHovered, { r , g , b , 1.f });

	std::string label = std::string(comp->GetTypeName()) + "##comp" + std::to_string(comp->GetId());
	bool open = m_gui->CollapsingHeader(label, false);

	m_gui->PopColor(2);

	HandleSelection(comp);

	if (m_gui->BeginContextMenu(std::to_string(comp->GetId()).c_str()))
	{
		if (m_gui->MenuItem("Delete"))
			m_pendingDelete = comp;
		m_gui->EndPopup();
	}

	if (!open) return;

	m_gui->PushStyleVariable(StyleVariable::IndentSpacing, 8.f);

	if (ScriptComponent* script = dynamic_cast<ScriptComponent*>(comp))
	{
		DrawScriptComponent(script);
	}
	else
	{
		for (auto& var : comp->GetExposedVariables())
			DrawVar(var, obj, comp);
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

	m_gui->PopStyleVariable();
}

void Apex::Editor::ComponentsViewer::HandleSelection(Apex::Component* comp)
{
	if (m_gui->IsItemClicked())
		m_selectedComp = comp;
}

void Apex::Editor::ComponentsViewer::DrawScriptComponent(Apex::Scripting::ScriptComponent* script)
{
	m_gui->AlignTextToFramePadding();
	m_gui->Text("Script:");
	m_gui->SameLine(LABEL_WIDTH);
	std::string name = script->GetScriptPath().empty() ? "None" : std::filesystem::path(script->GetScriptPath()).filename().string();
	m_gui->TextDisabled(name);

	float width = m_gui->GetAvailableSize()[0];
	if (m_gui->ButtonSized("Select Script##" + std::to_string(script->GetId()), { width, 22.f }))
		m_scriptPicker.Open(m_scene, script);

	// Exposed variables from Lua
	for (auto& var : script->GetExposedVariables())
	{
		switch (var.type)
		{
		case ExposedVar::Float:
		{
			m_gui->AlignTextToFramePadding();
			m_gui->Text(var.m_name);
			m_gui->SameLine(LABEL_WIDTH);
			m_gui->SetNextItemWidth(-1.f);
			float oldVal = std::get<float>(var.m_value);
			float newVal = oldVal;
			if (m_gui->InputFloat(std::string("##") + var.m_name, &newVal))
			{
				CommandManager::Get().Execute(std::make_unique<SetScriptVarCommand<float>>(m_scene, script, var.m_name, oldVal, newVal));
			}
			break;
		}
		case ExposedVar::Bool:
		{
			m_gui->AlignTextToFramePadding();
			m_gui->Text(var.m_name);
			m_gui->SameLine(LABEL_WIDTH);
			bool oldVal = std::get<bool>(var.m_value);
			bool newVal = oldVal;
			if (m_gui->Checkbox(std::string("##") + var.m_name, &newVal))
			{
				CommandManager::Get().Execute(std::make_unique<SetScriptVarCommand<bool>>(m_scene, script, var.m_name, oldVal, newVal));
			}
			break;
		}
		case ExposedVar::String:
		{
			m_gui->AlignTextToFramePadding();
			m_gui->Text(var.m_name);
			m_gui->SameLine(LABEL_WIDTH);
			m_gui->SetNextItemWidth(-1.f);
			char buffer[256];
			std::string const& oldVal = std::get<std::string>(var.m_value);
			strncpy_s(buffer, oldVal.c_str(), 255);
			buffer[255] = '\0';
			if (m_gui->InputText(std::string("##") + var.m_name, buffer, 256))
			{
				CommandManager::Get().Execute(std::make_unique<SetScriptVarCommand<std::string>>(
					m_scene, script, var.m_name, oldVal, std::string(buffer))
				);
			}
			break;
		}
		default: break;
		}
	}
}

void Apex::Editor::ComponentsViewer::DrawVar(ExposedVar& var, Object* obj, Apex::Component* comp)
{
	std::string wid = std::string("##") + var.m_name;
	switch (var.type)
	{
		case ExposedVar::Float:
		{
			m_gui->AlignTextToFramePadding();
			m_gui->Text(var.m_name);
			m_gui->SameLine(LABEL_WIDTH);
			m_gui->SetNextItemWidth(-1.f);
			float oldVal = *(float*)var.m_data;
			if (m_gui->InputFloat(wid, (float*)var.m_data))
			{
				m_changed = true;
				CommandManager::Get().Execute(
					std::make_unique<SetExposedVarCommand<float>>(var, obj, comp, oldVal, *(float*)var.m_data));
			}
			break;
		}
		case ExposedVar::Int:
		{
			m_gui->AlignTextToFramePadding();
			m_gui->Text(var.m_name);
			m_gui->SameLine(LABEL_WIDTH);
			m_gui->SetNextItemWidth(-1.f);
			int oldVal = *(int*)var.m_data;
			if (m_gui->InputInt(wid, (int*)var.m_data))
			{
				m_changed = true;
				CommandManager::Get().Execute(
					std::make_unique<SetExposedVarCommand<int>>(var, obj, comp, oldVal, *(int*)var.m_data));
			}
			break;
		}
		case ExposedVar::Bool:
		{
			m_gui->AlignTextToFramePadding();
			m_gui->Text(var.m_name);
			m_gui->SameLine(LABEL_WIDTH);
			bool oldVal = *(bool*)var.m_data;
			if (m_gui->Checkbox(wid, (bool*)var.m_data))
			{
				m_changed = true;
				CommandManager::Get().Execute(
					std::make_unique<SetExposedVarCommand<bool>>(var, obj, comp, oldVal, *(bool*)var.m_data));
			}
			break;
		}
		case ExposedVar::String:
		{
			m_gui->AlignTextToFramePadding();
			m_gui->Text(var.m_name);
			m_gui->SameLine(LABEL_WIDTH);
			m_gui->SetNextItemWidth(-1.f);
			std::string oldVal = *(std::string*)var.m_data;
			char buf[256];
			strncpy(buf, oldVal.c_str(), sizeof(buf));
			buf[sizeof(buf) - 1] = '\0'; // ensure null termination

			if (m_gui->InputText(wid, buf, sizeof(buf)))
			{
				m_changed = true;
				CommandManager::Get().Execute(
					std::make_unique<SetExposedVarCommand<std::string>>(var, obj, comp, oldVal, buf));
			}
			break;
		}
		case ExposedVar::Enum:
		{
			int oldVal = *(int*)var.m_data;
			if (DrawComponentEnum(var))
			{
				CommandManager::Get().Execute(
					std::make_unique<SetExposedVarCommand<int>>(var, obj, comp, oldVal, *(int*)var.m_data));
			}
			break;
		}
		case ExposedVar::Vector3:
		{
			LibMath::Vector3 oldVal = *(LibMath::Vector3*)var.m_data;
			if (DrawComponent(var.m_name, *(LibMath::Vector3*)var.m_data))
			{
				CommandManager::Get().Execute(
					std::make_unique<SetExposedVarCommand<LibMath::Vector3>>(var, obj, comp, oldVal, *(LibMath::Vector3*)var.m_data));
			}
			break;
		}
		case ExposedVar::Mesh:
			DrawComponent(var.m_name, (ResourceHandle<Model>*)var.m_data);
			break;
		case ExposedVar::Material:
			DrawComponent(var.m_name, (ResourceHandle<Material>*)var.m_data);
			break;
		case ExposedVar::Texture:
			DrawComponent(var.m_name, (ResourceHandle<Texture>*)var.m_data);
			break;
		case ExposedVar::Color3:
		{
			LibMath::Vector3 oldVal = *(LibMath::Vector3*)var.m_data;
			if (DrawComponentColor3(var))
			{
				CommandManager::Get().Execute(
					std::make_unique<SetExposedVarCommand<LibMath::Vector3>>(var, obj, comp, oldVal, *(LibMath::Vector3*)var.m_data));
			}
			break;
		}

		case ExposedVar::Color4:
		{
			LibMath::Vector4 oldVal = *(LibMath::Vector4*)var.m_data;
			if (DrawComponentColor4(var))
			{
				CommandManager::Get().Execute(
					std::make_unique<SetExposedVarCommand<LibMath::Vector4>>(var, obj, comp, oldVal, *(LibMath::Vector4*)var.m_data));
			}
			break;
		}
	}
}

bool Apex::Editor::ComponentsViewer::DrawComponentEnum(ExposedVar var)
{
	m_gui->AlignTextToFramePadding();
	m_gui->Text(var.m_name);
	m_gui->SameLine(LABEL_WIDTH);
	m_gui->SetNextItemWidth(-1.f);

	int value = *(int*)var.m_data;
	std::string items;
	for (const auto& n : var.m_enumNames) items += n + '\0';
	if (m_gui->Combo(std::string("##") + var.m_name, &value, items.c_str()))
	{
		*(int*)var.m_data = value; 
		m_changed = true;
		return true;
	}
	return false;
}

bool Apex::Editor::ComponentsViewer::DrawComponent(std::string const& name, LibMath::Vector3& data)
{
	if (DrawVec3(name, data, -1000.f, 1000.f))
	{
		m_changed = true;
		return true;
	}
	return false;
}

bool Apex::Editor::ComponentsViewer::DrawComponentColor3(ExposedVar var)
{
	m_gui->AlignTextToFramePadding();
	m_gui->Text(var.m_name);
	m_gui->SameLine(LABEL_WIDTH);
	if (m_gui->ColorEdit3("##c3", (float*)var.m_data))
	{
		m_changed = true;
		return true;
	}
	return false;
}

bool Apex::Editor::ComponentsViewer::DrawComponentColor4(ExposedVar var)
{
	m_gui->AlignTextToFramePadding();
	m_gui->Text(var.m_name);
	m_gui->SameLine(LABEL_WIDTH);
	if (m_gui->ColorEdit4("##c4", (float*)var.m_data))
	{
		m_changed = true;
		return true;
	}
	return false;
}

void Apex::Editor::ComponentsViewer::DrawComponent(std::string const& name, ResourceHandle<Model>* mesh)
{
	uint32_t thumb = mesh->IsValid() ? ResolveMeshPreview((*mesh)->GetPath()) : 0;
	DrawAssetPreview(thumb, "Mesh", PREVIEW_SIZE);
	m_gui->SameLine(0.f, 6.f);

	m_gui->BeginChildPanel(std::string("##meshcol") + name, 0.f, PREVIEW_SIZE, false);
	m_gui->TextDisabled(mesh->IsValid() ? std::filesystem::path((*mesh)->GetPath()).filename().string() : "None");

	float width = m_gui->GetAvailableSize()[0];
	if (m_gui->ButtonSized(std::string("Select##") + name, { width, 22.f }))
		m_meshPicker.Open(mesh);
	m_gui->EndChildPanel();
}

void Apex::Editor::ComponentsViewer::DrawComponent(std::string const& name, ResourceHandle<Material>* material)
{
	uint32_t thumb = material->IsValid() ? ResolveMaterialPreview((*material)->GetPath()) : 0;
	DrawAssetPreview(thumb, name, PREVIEW_SIZE);
	m_gui->SameLine(0.f, 6.f);

	m_gui->BeginChildPanel(std::string("##matcol") + name, 0.f, PREVIEW_SIZE, false);
	m_gui->TextDisabled(material->IsValid() ? std::filesystem::path((*material)->GetPath()).filename().string() : "None");

	float width = m_gui->GetAvailableSize()[0];
	if (m_gui->ButtonSized(std::string("Select##") + name, { width, 22.f }))
		m_materialPicker.Open(material);
	m_gui->EndChildPanel();
}

void Apex::Editor::ComponentsViewer::DrawComponent(std::string const& name, ResourceHandle<Texture>* texture)
{
	uint32_t thumb = texture->IsValid() ? ResolveTexturePreview(texture) : 0;
	DrawAssetPreview(thumb, "Tex", PREVIEW_SIZE);
	m_gui->SameLine(0.f, 6.f);

	m_gui->BeginChildPanel(std::string("##texcol") + name, 0.f, PREVIEW_SIZE, false);
	m_gui->TextDisabled(texture->IsValid() ? std::filesystem::path((*texture)->GetPath()).filename().string() : "None");

	float width = m_gui->GetAvailableSize()[0];
	if (m_gui->ButtonSized(std::string("Select##") + name, { width, 22.f }))
		m_texturePicker.Open(texture);
	m_gui->EndChildPanel();
}

uint32_t Apex::Editor::ComponentsViewer::ResolveTexturePreview(Apex::Resources::ResourceHandle<Texture>* handle) const
{
	if (!handle || !handle->IsValid()) return 0;
	return EditorIcon::Get((*handle)->GetPath());
}

uint32_t Apex::Editor::ComponentsViewer::ResolveMeshPreview(const std::string& path) const
{
	if (path.empty() || !m_thumbnailRenderer) return 0;
	return m_thumbnailRenderer->GetOrRender(path);
}

uint32_t Apex::Editor::ComponentsViewer::ResolveMaterialPreview(const std::string& path) const
{
	if (path.empty() || !m_thumbnailRenderer) return 0;
	return m_thumbnailRenderer->GetOrRenderMaterial(path);
}

void Apex::Editor::ComponentsViewer::DrawAssetPreview(uint32_t texID, const std::string& fallbackLabel, float size)
{
	m_gui->InvisibleButton(std::string("##prev") + fallbackLabel, { size, size });
	LibMath::Vector2 bMin = m_gui->GetItemRectMin();
	LibMath::Vector2 bMax = m_gui->GetItemRectMax();

	IDrawList* drawList = m_gui->GetDrawList();

	drawList->DrawRectFilled(bMin, bMax, PackColor(25, 25, 28), 4.f);
	drawList->DrawRect(bMin, bMax, PackColor(80, 80, 85), 4.f, 1.f);

	if (texID != 0)
	{
		LibMath::Vector2 afterBtn = bMax;
		m_gui->SetCursorPos(bMin);
		m_gui->DrawImageTinted(texID, { size, size }, { 1.f, 1.f, 1.f, 1.f }, true);
		m_gui->SetCursorPos({ afterBtn[0], bMin[1] });
	}
	else
	{
		LibMath::Vector2 tsz = drawList->CalculateTextSize(fallbackLabel);
		drawList->DrawText({ bMin[0] + (size - tsz[0]) * 0.5f, bMin[1] + (size - tsz[1]) * 0.5f }, 0xFF888888, fallbackLabel);
	}
}
