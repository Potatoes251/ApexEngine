#include "Viewport.h"

#include <LibMath/Arithmetic.h>
#include <LibMath/Colision3D.h>

#include "Command/TransformCommand.h"
#include "Command/SetValCommand.h"
#include "CommandManager.h"
#include "InputSystem.h"
#include "Hierarchy.h"
#include "Editor.h"
#include "Window.h"
#include "Scene.h"
#include "RHI.h"
#include "Hud.h"
#include "EditorIcon.h"

#include "WaypointGraph.h"
#include "Waypoint.h"

using namespace Apex::Data;

using namespace Apex::Editor;
using namespace Apex::Rendering;
using namespace Apex::Input;
using namespace Apex::CtrlZ;

Viewport::Viewport(Resources::ResourceHandle<Shader> pickingShader, Resources::ResourceHandle<Shader> gridShader,
	Rendering::IRHI* rhi, UserInterface::IGUI* gui, Editor* editor)
	: m_gizmo(pickingShader, rhi), m_gridShader(gridShader), m_rhi(rhi), m_gui(gui) 
{
	m_fbo = m_rhi->CreateFrameBuffer(m_width, m_height, m_textureFbo);
	m_pickingshader = pickingShader;
	m_editor = editor;
	m_camera.SetProjectionMatrix(m_width / m_height);
}

Viewport::~Viewport()
{
	m_rhi->DeleteFrameBuffer(m_fbo);
	m_rhi->DeleteTexture(m_textureFbo);
}

void Viewport::SelectObject(Data::Object* selected, bool send)
{
	m_selectedObject = selected;
	if (send && m_hierarchy)
		m_hierarchy->SelectObject(selected, false);
}

void Viewport::UpdateCamera(Windowing::IWindow* window)
{
	if (!m_isFocus || m_editor->GetEditorState() == EditorState::Play) return;

	InputSystem& input = InputSystem::Get();

	// dont move while using shortcut
	if (input.IsKeyDown(Key::Ctrl)) return;

	m_camera.ProcessKeyboard(input.IsKeyDown(Key::W), input.IsKeyDown(Key::S), 
		input.IsKeyDown(Key::A), input.IsKeyDown(Key::D), 
		input.IsKeyDown(Key::E), input.IsKeyDown(Key::Q), window->GetDeltaTime());
}

void Viewport::OnMouseClick(double mouseX, double mouseY)
{
	if (!m_isFocus || m_editor->GetEditorState() != EditorState::Edit
	    || mouseX < m_panelPos[0] || mouseX >  m_panelPos[0] + m_panelSize[0]
		|| mouseY < m_panelPos[1] || mouseY >  m_panelPos[1] + m_panelSize[1])
		return;

	PerformPicking({ (float)mouseX, (float)mouseY });
	CommandManager::Get().StartBatch();
}

void Viewport::OnMouseRelease(double /*mouseX*/, double /*mouseY*/)
{
	if (!m_isFocus) return;
	m_selectedAxis = Axis::None;
	CommandManager::Get().EndBatch();
}

void Viewport::OnMouseMove(double mouseX, double mouseY)
{
	if (m_selectedAxis == Axis::None || !m_selectedObject)
		return;

	const LibMath::Vector3 axis = GetAxis();

	LibMath::Matrix4 viewProj = m_camera.GetViewProj();

	LibMath::Line3D ray = ScreenToWorldRay({ (float)mouseX, (float)mouseY }, viewProj, m_panelPos, m_panelSize);

	LibMath::Vector3 hit;
	if (!LibMath::intersectionRayPlane(ray, m_axisPlane, hit)) return;

	LibMath::Vector3 delta = hit - m_startingPoint;
	LibMath::Vector3 movementVec;

	if (IsAxisSingle())
	{
		float movement = delta.dot(axis);
		movementVec = axis * movement;
	}
	else if (IsAxisPlane() || IsAxisFree())
	{
		movementVec = delta;
	}

	LibMath::Transform newTransform = m_selectedObject->GetLocalTransform();

	switch (m_currentGizmoMode)
	{
	case GizmoMode::Translation:
	{
		if (m_isTranslationSnapping) Snap(movementVec);
		newTransform.setPosition(m_initialTransform.getPosition() + movementVec);
		CommandManager::Get().Execute(std::make_unique<TransformCommand>(m_selectedObject, newTransform, false));
		break;
	}
	case GizmoMode::Rotation:
	{
		assert(IsAxisSingle() && "Rotation are only allowed around a single axis");
		LibMath::Vector3 center = m_initialTransform.getPosition();

		LibMath::Vector3 startVec = (m_startingPoint - center).normalize();
		LibMath::Vector3 currentVec = (hit - center).normalize();

		float angle = atan2(axis.dot(startVec.cross(currentVec)), startVec.dot(currentVec));

		if (m_isRotationSnapping && m_rotateSnap > 0.f) Snap(angle, m_rotateSnap);

		LibMath::Quaternion deltaAngle(LibMath::Radian(angle), axis);

		newTransform.setRotation(deltaAngle * m_initialTransform.getRotation());
		CommandManager::Get().Execute(std::make_unique<TransformCommand>(m_selectedObject, newTransform, false));
		break;
	}
	case GizmoMode::Scale:
	{
		LibMath::Vector3 scaleDelta = movementVec;
		if (IsAxisFree())
		{
			float movement = delta.dot(m_camera.GetRight()) + delta.dot(m_camera.GetUp());
			scaleDelta = { movement, movement, movement };
		}
		if (m_isScaleSnapping) Snap(scaleDelta);
		LibMath::Vector3 newScale = m_initialTransform.getScale() + scaleDelta;
		newScale = LibMath::Vector3(std::max(newScale[0], 0.01f), std::max(newScale[1], 0.01f), std::max(newScale[2], 0.01f));

		newTransform.setScale(newScale);
		CommandManager::Get().Execute(std::make_unique<TransformCommand>(m_selectedObject, newTransform, false));
		break;
	}
	default:
		assert(false && "Unhandled gizmo mode");
		break;
	}
}

void Viewport::ToggleFullScreen()
{
	m_isFullScreen = !m_isFullScreen;
	m_isFirstFullScreen = true;
}

int Viewport::GetMousePixelInFbo(float mouseX, float mouseY)
{
	// relative to panel top-left
	float localX = mouseX - m_panelPos[0];
	float localY = mouseY - m_panelPos[1];

	// clamp to panel
	localX = LibMath::clamp(localX, 0.0f, m_panelSize[0]);
	localY = LibMath::clamp(localY, 0.0f, m_panelSize[1]);

	float scaleX = float(m_width) / m_panelSize[0];
	float scaleY = float(m_height) / m_panelSize[1];

	float fboX = localX * scaleX;
	float fboY = localY * scaleY;
	fboY = m_height - fboY;

	return m_rhi->GetPixelData((int)fboX, (int)fboY);
}

void Viewport::PerformPicking(LibMath::Vector2 mousePos)
{
	if (!m_pickingshader.IsReady()) return;

	m_rhi->BindFrameBuffer(m_fbo, m_width, m_height);
	m_rhi->Clear();

	LibMath::Matrix4 view = m_camera.GetViewMatrix();
	LibMath::Matrix4 proj = m_camera.GetProjection();
	m_rhi->SetCullingEnabled(false);
	m_gizmo.DrawGizmos(m_selectedObject, m_currentGizmoMode, proj * view, m_camera.GetPosition(), true, m_localGizmo);
	m_rhi->SetCullingEnabled(true);

	int id = GetMousePixelInFbo(mousePos[0], mousePos[1]);

	if (id >= static_cast<int>(Axis::X) && id < static_cast<int>(Axis::None))
	{
		m_selectedAxis = static_cast<Axis>(id);

		m_initialTransform = m_selectedObject->GetGlobalTransform();

		LibMath::Line3D ray = ScreenToWorldRay(mousePos, m_camera.GetViewProj(), m_panelPos, m_panelSize);

		if (m_currentGizmoMode != GizmoMode::Rotation) ComputePlane();
		else ComputePlaneRotation();

		if (LibMath::intersectionRayPlane(ray, m_axisPlane, m_startingPoint))
		{
			if (m_currentGizmoMode != GizmoMode::Rotation)
			{
				LibMath::Vector3 center = m_selectedObject->GetGlobalTransform().getPosition();
				m_startingPoint = center + GetAxis() * ((m_startingPoint - center).dot(GetAxis()));
			}

			m_rhi->UnBindFrameBuffer(m_width, m_height);
			return;
		}
	}

	m_selectedAxis = Axis::None;

	RenderPassInfo picking;
	picking.m_aspectRatio = m_width / m_height;
	picking.m_camera = &m_camera;
	picking.m_flags |= UseIdAsColor;
	picking.m_flags |= IgnoreMaterial;
	picking.m_flags &= ~RenderSkybox;
	picking.m_shader = m_pickingshader;
	picking.m_target = m_fbo;
	picking.m_rhi = m_rhi;

	m_pickingshader->Use();
	m_editor->GetApp()->GetScene()->Render(picking);

	SelectObject(m_editor->GetApp()->GetScene()->GetObjectWithId(GetMousePixelInFbo(mousePos[0], mousePos[1])));

	m_rhi->UnBindFrameBuffer(m_width, m_height);
}

LibMath::Vector3 Viewport::GetAxis()
{
	LibMath::Vector3 axis;
	switch (m_selectedAxis)
	{
	case Axis::X:
		axis = { 1, 0, 0 };	
		break;
	case Axis::Y:
		axis = { 0, 1, 0 };	
		break;
	case Axis::Z:
		axis = { 0, 0, 1 };	
		break;
	case Axis::None:	return { 0, 0, 0 };
	default:			return { 0, 0, 0 };
	}

	if (!m_localGizmo || !m_selectedObject) return axis;

	return m_initialTransform.getRotation().rotate(axis);
}

LibMath::Vector3 Apex::Editor::Viewport::GetAxisPlane()
{
	LibMath::Vector3 axis;

	switch (m_selectedAxis)
	{
	case Axis::XY:		
		axis = { 0, 0, 1 };
		break;
	case Axis::XZ:		
		axis = { 0, 1, 0 };
		break;
	case Axis::YZ:		
		axis = { 1, 0, 0 };
		break;
	case Axis::XYZ:		
		axis = m_camera.GetFront();
		break;
	case Axis::None:	return { 0, 0, 0 };
	default:			return { 0, 0, 0 };
	}

	if (!m_localGizmo || !m_selectedObject) return axis;

	return m_selectedObject->GetGlobalTransform().getRotation().rotate(axis); 
}

bool Apex::Editor::Viewport::IsAxisSingle() const
{
	return m_selectedAxis == Axis::X || m_selectedAxis == Axis::Y || m_selectedAxis == Axis::Z;
}

bool Apex::Editor::Viewport::IsAxisPlane() const
{
	return m_selectedAxis == Axis::XY || m_selectedAxis == Axis::XZ || m_selectedAxis == Axis::YZ;
}

bool Apex::Editor::Viewport::IsAxisFree() const
{
	return m_selectedAxis == Axis::XYZ;
}

void Viewport::ComputePlane()
{
	if (m_selectedAxis == Axis::None || !m_selectedObject) return;

	LibMath::Vector3 planeNormal;

	if (IsAxisSingle())
	{
		LibMath::Vector3 axis = GetAxis();

		planeNormal = m_camera.GetFront() - axis * m_camera.GetFront().dot(axis);

		if (planeNormal.magnitudeSquared() < g_epsilon)
			planeNormal = axis.cross(m_camera.GetRight());
	}
	else if (IsAxisPlane())
	{
		planeNormal = GetAxisPlane(); // fixed plane
	}
	else if (IsAxisFree())
	{
		planeNormal = m_camera.GetFront();
	}

	m_axisPlane = LibMath::Plane(planeNormal, -planeNormal.dot(m_selectedObject->GetGlobalTransform().getPosition()));
}

void Viewport::ComputePlaneRotation()
{
	if (m_selectedAxis == Axis::None || !m_selectedObject) return;

	LibMath::Vector3 planeNormal = GetAxis();

	m_axisPlane = LibMath::Plane(planeNormal, -planeNormal.dot(m_selectedObject->GetGlobalTransform().getPosition()));
}

void Viewport::Snap(float& value, float snapValue) const
{
	value = round(value / snapValue) * snapValue;
}

void Viewport::Snap(LibMath::Vector3& value) const
{
	switch (m_currentGizmoMode)
	{
	case Apex::Editor::GizmoMode::Translation:
		if (m_translateSnap > 0.0f)
		{
			Snap(value[0], m_translateSnap);
			Snap(value[1], m_translateSnap);
			Snap(value[2], m_translateSnap);
		}
		break;
	case Apex::Editor::GizmoMode::Rotation:
		if (m_rotateSnap > 0.0f)
		{
			Snap(value[0], m_rotateSnap);
			Snap(value[1], m_rotateSnap);
			Snap(value[2], m_rotateSnap);
		}
		break;
	case Apex::Editor::GizmoMode::Scale:
		if (m_scaleSnap > 0.0f)
		{
			Snap(value[0], m_scaleSnap);
			Snap(value[1], m_scaleSnap);
			Snap(value[2], m_scaleSnap);
		}
		break;
	default:
		break;
	}
}

void Viewport::Draw()
{
	float aspectRatio = m_width / m_height;

	RenderPassInfo pass;
	pass.m_aspectRatio = aspectRatio;
	pass.m_target = m_fbo;
	pass.m_rhi = m_rhi;
	if (m_showColliders)
		pass.m_flags |= ShowCollider;

	if (m_editor->GetEditorState() == EditorState::Play)
		pass.m_camera = m_editor->GetApp()->GetScene()->GetMainCamera();
	else
		pass.m_flags &= ~PerformCompute;
	if (!pass.m_camera)
		pass.m_camera = &m_camera;

	if (m_isFirstFullScreen)
	{
		m_isFirstFullScreen = false;
		if (m_isFullScreen)
		{
			LibMath::Vector2 screen = m_gui->GetScreenSize();
			m_gui->SetNextWindowPos(LibMath::Vector2(0));
			m_gui->SetNextWindowSize(LibMath::Vector2(screen[0], screen[1]));
			m_gui->SetNextWindowFocus();
			m_oldPanelPos = m_panelPos;
			m_oldPanelSize = m_panelSize;
		}
		else
		{
			m_gui->SetNextWindowPos(m_oldPanelPos);
			m_gui->SetNextWindowSize(m_oldPanelSize);
		}
	}

	m_gui->BeginPanel("Viewport");

	m_isFocus = m_gui->IsPanelFocused();
	m_panelPos = m_gui->GetPanelPos();
	m_panelSize = m_gui->GetPanelSize();

	m_editor->GetApp()->Render(m_gui, pass);
	if (m_editor->GetEditorState() == EditorState::Edit)
	{
		if (m_showGrid) DrawGrid();
		m_rhi->ClearDepth();

		LibMath::Matrix4 view = m_camera.GetViewMatrix();
		LibMath::Matrix4 proj = m_camera.GetProjection();
		m_rhi->SetCullingEnabled(false);
		m_gizmo.DrawGizmos(m_selectedObject, m_currentGizmoMode, proj * view, m_camera.GetPosition(), false, m_localGizmo);
		m_rhi->SetCullingEnabled(true);
	}
	m_rhi->UnBindFrameBuffer(m_width, m_height);

	float panelRatio = m_panelSize[0] / m_panelSize[1];
	LibMath::Vector2 drawSize;
	LibMath::Vector2 offset;

	if (panelRatio > aspectRatio)
	{
		drawSize[1] = m_panelSize[1];
		drawSize[0] = drawSize[1] * aspectRatio;
		offset[0] = (m_panelSize[0] - drawSize[0]) * 0.5f;
	}
	else
	{
		drawSize[0] = m_panelSize[0];
		drawSize[1] = drawSize[0] / aspectRatio;
		offset[1] = (m_panelSize[1] - drawSize[1]) * 0.5f;
	}

	m_gui->DrawImageBackground(m_rhi->GetTexture(m_textureFbo), m_panelPos + offset, m_panelPos + offset + drawSize);
	m_rhi->UnBindFrameBuffer();

	if (m_editor->GetEditorState() != EditorState::Edit)
	{
		auto* uiManager = m_editor->GetApp()->GetUI();
		if (uiManager)
			uiManager->RenderHUD(m_gui->GetDrawList(), m_panelPos + offset, drawSize);
	}

	DrawUI();

	DrawBuildPopup();

	m_gui->EndPanel();
}

void Viewport::DrawUI()
{
	DrawPlayBar();
	if (m_editor->GetEditorState() == EditorState::Edit)
		DrawRightToolbar();

	DrawCameraSpeed();
}

void Viewport::DrawGrid()
{
	if (!m_gridShader.IsReady()) return;
	m_gridShader->Use();

	LibMath::Matrix4 view = m_camera.GetViewMatrix();
	LibMath::Matrix4 proj = m_camera.GetProjection();
	LibMath::Matrix4 invViewProj = proj * view;
	m_gridShader->SetUniform("uViewProj", invViewProj);
	invViewProj.inverse();
	m_gridShader->SetUniform("uInvViewProj", invViewProj);

	m_gridShader->SetUniform("uCameraPos", m_camera.GetPosition());

	m_rhi->DrawEmpty();
}

void Apex::Editor::Viewport::DrawPlayBar()
{
	EditorState state = m_editor->GetEditorState();

	constexpr float BUTTON = 36.f;
	constexpr float PAD = 4.f;
	constexpr float ROUND = BUTTON * 0.5f;

	auto drawBar = [&](float count)
		{
			float groupWidth = count * (BUTTON + PAD) - PAD + PAD * 7.f;
			float startX = m_panelPos[0] + (m_panelSize[0] - groupWidth) * 0.5f;
			float startY = m_panelPos[1] + 30.f;
			DrawPillGroup({ startX - 2.f, startY - 2.f }, { groupWidth, BUTTON + PAD * 4.8f }, ROUND);
			m_gui->SetCursorPos({ startX + PAD, startY + PAD * 0.5f });
		};

	if (state == EditorState::Edit)
	{
		m_gui->PushFont(FontID::Default);
		DrawPillGroup({ m_panelPos[0] + 4.f, m_panelPos[1] + 27.5f }, { 125.f, BUTTON + PAD * 2.f }, ROUND);
		if (m_gui->Button("Build Game"))
		{
			// Scan for .level files once when the popup opens
			m_buildScenes.clear();
			namespace Fs = std::filesystem;
			for (auto& entry : Fs::recursive_directory_iterator("Assets/"))
			{
				if (entry.path().extension() == ".level")
				{
					BuildSceneEntry sceneEntry;
					sceneEntry.path = entry.path().generic_string();
					sceneEntry.selected = true;
					m_buildScenes.push_back(std::move(sceneEntry));
				}
			}
			m_showBuildPopup = true;
			m_gui->OpenPopup("Build Game");
		}
		DrawPillGroup({ m_panelPos[0] + 4.f, m_panelPos[1] + 64.f }, { 187.f, BUTTON + PAD * 2.f }, ROUND);
		if (m_gui->Button("GenerateNavGraph"))
		{
			Scene* scene = m_editor->GetApp()->GetScene();
			std::filesystem::path path{ m_editor->GetApp()->GetScene()->GetName()};
			path.replace_extension(".nav");
			std::vector<Pathfinding::NavGenVolume*> navVol = scene->GetComponents<Pathfinding::NavGenVolume>();
			if (!navVol.empty())
				scene->GetGraph()->GenerateGraph(path.string(), scene->GetPhysic(), navVol[0]);
		}

		m_gui->PopFont();
		drawBar(1); // only Play
		if (IconButton("ApexAssets/Icons/play.png", ">", { BUTTON, BUTTON }, false, Apex::UserInterface::Color(0.0f, 1.0f, 0.0f, 1.0f)))
			m_editor->SetEditorState(EditorState::Play);
	}
	else
	{
		// Show Pause/Resume + Step + Stop in a pill group
		bool playing = (state == EditorState::Play);
		bool paused = (state == EditorState::Pause);

		drawBar(3.8f);

		if (IconButton("ApexAssets/Icons/pause.png", "||", { BUTTON, BUTTON }, paused))
		{
			if (playing) m_editor->SetEditorState(EditorState::Pause);
			else if (paused) m_editor->SetEditorState(EditorState::Play);
		}
		m_gui->SameLine(0.f, PAD);

		if (IconButton("ApexAssets/Icons/step.png", "|>", { BUTTON, BUTTON }))
			if (paused) m_editor->SetEditorState(EditorState::Step);
		m_gui->SameLine(0.f, PAD);

		if (IconButton("ApexAssets/Icons/stop.png", "[]", { BUTTON, BUTTON }, false, Apex::UserInterface::Color(1.0f, 0.0f, 0.0f, 1.0f)))
		{
			m_editor->SetEditorState(EditorState::Edit);
			SelectObject(nullptr);
			if (m_isFullScreen) 
			{ 
				m_isFullScreen = false; 
				m_isFirstFullScreen = true; 
			}
		}
	}
}

void Apex::Editor::Viewport::DrawRightToolbar()
{
	constexpr float BUTTON = 36.f;
	constexpr float PAD = 4.f;
	constexpr float ROUND = BUTTON * 0.5f;
	constexpr float GROUP_X_OFFSET = 20.f; // padding from right edge

	float rightX = m_panelPos[0] + m_panelSize[0] - BUTTON - PAD * 2.f - GROUP_X_OFFSET;
	float curY = m_panelPos[1] + 32.f;

	float groupHeight = 4.95f * (BUTTON + PAD) - PAD + PAD * 2.f;
	DrawPillGroup({ rightX - PAD, curY - 6.f }, { BUTTON + PAD * 6.f, groupHeight }, ROUND);

	auto gizmoButton = [&](const char* icon, const char* label, GizmoMode mode)
		{
			m_gui->SetCursorPos({ rightX, curY });
			if (IconButton(icon, label, { BUTTON, BUTTON }, m_currentGizmoMode == mode))
				m_currentGizmoMode = mode;
			curY += BUTTON + PAD * 3.f;
		};

	gizmoButton("ApexAssets/Icons/translate.png", "+", GizmoMode::Translation);
	gizmoButton("ApexAssets/Icons/rotate.png", "O", GizmoMode::Rotation);
	gizmoButton("ApexAssets/Icons/scale.png", "#", GizmoMode::Scale);

	m_gui->SetCursorPos({ rightX, curY });
	if (IconButton(m_localGizmo ? "ApexAssets/Icons/local.png" : "ApexAssets/Icons/world.png", m_localGizmo ? "L" : "W", { BUTTON, BUTTON }))
		m_localGizmo = !m_localGizmo;
	if (m_gui->IsItemHovered())
	{
		m_gui->BeginTooltip();
		m_gui->Text(m_localGizmo ? "Local Space" : "World Space");
		m_gui->EndTooltip();
	}
	curY += BUTTON + PAD * 6.f;

	float group2Height = 2.55f * (BUTTON + PAD) - PAD + PAD * 2.f;
	DrawPillGroup({ rightX - PAD, curY - 6.f }, { BUTTON + PAD * 6.f, group2Height }, ROUND);

	auto viewButton = [&](const char* icon, const char* label, bool& toggle, const char* tooltip)
		{
			m_gui->SetCursorPos({ rightX, curY });
			if (IconButton(icon, label, { BUTTON, BUTTON }, toggle))
				toggle = !toggle;
			if (m_gui->IsItemHovered())
			{
				m_gui->BeginTooltip();
				m_gui->Text(tooltip);
				m_gui->EndTooltip();
			}
			curY += BUTTON + PAD * 3.f;
		};

	viewButton("ApexAssets/Icons/grid.png", "G", m_showGrid, "Toggle Grid");
	viewButton("ApexAssets/Icons/collider.png", "C", m_showColliders, "Toggle Colliders");

	curY += PAD * 5.f; 

	DrawSnapButtons(BUTTON, PAD, rightX, curY, ROUND);
}

void Apex::Editor::Viewport::DrawCameraSpeed()
{
	if (!InputSystem::Get().IsMouseButtonDown(Input::MouseButton::Right) ||
		m_editor->GetEditorState() == EditorState::Play)
		return;

	constexpr float PAD = 4.f;
	constexpr float HEIGHT = 36.f;
	constexpr float ROUND = HEIGHT * 0.5f;

	float speed = m_camera.GetSpeed();

	char buf[32];
	snprintf(buf, sizeof(buf), "Speed: %.1f", speed);

	// Position
	LibMath::Vector2 pos =
	{
		m_panelPos[0] + 5.f,
		m_panelPos[1] + 105.f
	};

	float backgroundWidth = 110.f;

	// Add 10.f for every extra digit after the first.
	// 0-9   -> 110.f
	// 10-99 -> 120.f
	// 100+  -> 130.f

	float absSpeed = std::abs(speed);

	while (absSpeed >= 10.f)
	{
		backgroundWidth += 10.f;
		absSpeed /= 10.f;
	}

	// Background size
	LibMath::Vector2 size =
	{
		backgroundWidth,
		HEIGHT
	};

	// Draw pill background
	DrawPillGroup(pos, size, ROUND);

	// Draw text inside
	m_gui->PushFont(FontID::Default);

	m_gui->SetCursorPos({
		pos[0] + PAD * 3.f,
		pos[1] + PAD * 1.5f
		});

	m_gui->Text(buf);

	m_gui->PopFont();
}

void Apex::Editor::Viewport::DrawSnapButtons(const float button, const float pad, float rightX, float& curY, const float round)
{
	float SNAP_BUTTON_WIDTH = button;
	float SNAP_INPUT_WIDTH = 60.f;
	float SNAP_CELL_HEIGHT = button + pad * 3.f;
	float snapGroupWidth = SNAP_BUTTON_WIDTH + pad + SNAP_INPUT_WIDTH + pad * 6.5f;
	float snapGroupHeight = 3.f * SNAP_CELL_HEIGHT - pad + pad * 4.f;
	float snapLeft = rightX + button + pad * 4.f - snapGroupWidth;
	DrawPillGroup({ snapLeft + 2.f, curY - 6.f }, { snapGroupWidth, snapGroupHeight }, round);

	struct SnapEntry
	{
		const char* m_icon;
		const char* m_label;
		bool* m_enabled;
		float* m_value;
	};

	SnapEntry snaps[3] =
	{
		{ "ApexAssets/Icons/translate_snap.png", "Translate", &m_isTranslationSnapping, &m_translateSnap },
		{ "ApexAssets/Icons/rotate_snap.png",    "Rotate", &m_isRotationSnapping,    &m_rotateSnap },
		{ "ApexAssets/Icons/scale_snap.png",     "Scale", &m_isScaleSnapping,       &m_scaleSnap },
	};

	for (auto& element : snaps)
	{
		float rowX = snapLeft + pad * 2.f;
		m_gui->SetCursorPos({ rowX, curY + pad * 0.5f });

		if (IconButton(element.m_icon, element.m_label, { SNAP_BUTTON_WIDTH, SNAP_BUTTON_WIDTH }, *element.m_enabled))
		{
			CtrlZ::CommandManager::Get().Execute(std::make_unique<CtrlZ::SetValCommand<bool>>(
				element.m_enabled, *element.m_enabled, !*element.m_enabled)
			);
		}

		if (m_gui->IsItemHovered())
		{
			m_gui->BeginTooltip();
			m_gui->Text(std::string(*element.m_enabled ? "Snapping ON  — " : "Snapping OFF — ") + element.m_label);
			m_gui->EndTooltip();
		}
		m_gui->SameLine(0.f, pad);
		m_gui->SetNextItemWidth(SNAP_INPUT_WIDTH);
		m_gui->PushFont(FontID::Default);
		float val = *element.m_value;
		if (m_gui->InputFloat(std::string("##sv") + element.m_label, &val))
		{
			CtrlZ::CommandManager::Get().Execute(std::make_unique<CtrlZ::SetValCommand<float>>(element.m_value, *element.m_value, val));
		}
		m_gui->PopFont();
		curY += SNAP_CELL_HEIGHT;
	}
}

void Apex::Editor::Viewport::DrawPillGroup(LibMath::Vector2 topLeft, LibMath::Vector2 size, float rounding) const
{
	IDrawList* drawList = m_gui->GetDrawList();
	LibMath::Vector2 bottomRight = { topLeft[0] + size[0], topLeft[1] + size[1] };
	drawList->DrawRectFilled(topLeft, bottomRight, 0xCC1A1A1A, rounding);
	drawList->DrawRect(topLeft, bottomRight, 0x66888888, rounding, 1.f);
}

bool Apex::Editor::Viewport::IconButton(const char* iconPath, const char* fallbackLabel, LibMath::Vector2 size, bool active, 
										Apex::UserInterface::Color tint)
{
	uint32_t icon = EditorIcon::Get(iconPath);

	if (active)
		m_gui->PushColor(StyleColor::Button, { 0.26f, 0.59f, 0.98f, 1.f });

	bool clicked = false;
	if (icon != 0)
		clicked = m_gui->ImageButton(fallbackLabel, icon, size, tint);
	else
		clicked = m_gui->ButtonSized(fallbackLabel, size);

	if (active)
		m_gui->PopColor();

	return clicked;
}

void Viewport::DrawBuildPopup()
{
	if (!m_showBuildPopup) return;

	m_gui->PushFont(FontID::Default);
	if (!m_gui->BeginPopupModal("Build Game", { 500.f, 0.f }))
		return;

	m_gui->Text("Select scenes to include in the build.");
	m_gui->Text("The first checked scene will be loaded on startup.");
	m_gui->Separator();

	for (int i = 0; i < (int)m_buildScenes.size(); ++i)
	{
		auto& entry = m_buildScenes[i];

		// Checkbox
		m_gui->Checkbox(("##sel" + std::to_string(i)).c_str(), &entry.selected);
		m_gui->SameLine();

		// Path label (just the filename for readability, full path as tooltip)
		std::string filename = std::filesystem::path(entry.path).filename().string();
		m_gui->Text(filename + "  (" + entry.path + ")");

		// Reorder buttons
		m_gui->SameLine();
		m_gui->PushID(i);
		if (i > 0 && m_gui->Button("^"))
			std::swap(m_buildScenes[i], m_buildScenes[i - 1]);
		m_gui->SameLine();
		if (i < (int)m_buildScenes.size() - 1 && m_gui->Button("v"))
			std::swap(m_buildScenes[i], m_buildScenes[i + 1]);
		m_gui->PopID();
	}

	if (m_buildScenes.empty())
		m_gui->Text("(No .level files found under Assets/)");

	m_gui->Separator();

	bool anySelected = false;
	for (auto& entry : m_buildScenes)
	{
		if (entry.selected) 
		{ 
			anySelected = true; 
			break; 
		}
	}

	if (!anySelected) m_gui->BeginDisabled();
	if (m_gui->Button("Build"))
	{
		std::vector<std::string> selected;
		for (auto& e : m_buildScenes)
			if (e.selected) selected.push_back(e.path);

		m_editor->Build(selected);
		m_showBuildPopup = false;
		m_gui->CloseCurrentPopup();
	}
	if (!anySelected) m_gui->EndDisabled();

	m_gui->SameLine();

	if (m_gui->Button("Cancel"))
	{
		m_showBuildPopup = false;
		m_gui->CloseCurrentPopup();
	}
	m_gui->PopFont();
	m_gui->EndPopup();
}