#include "Viewport.h"

#include <LibMath/Arithmetic.h>
#include <LibMath/Colision3D.h>

#include "Command/TransformCommand.h"
#include "CommandManager.h"
#include "InputSystem.h"
#include "Hierarchy.h"
#include "Editor.h"
#include "Window.h"
#include "Scene.h"
#include "RHI.h"
#include "UI.h"

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

void Viewport::SelectObject(Data::Object* selected, bool send)
{
	m_selectedObject = selected;
	if (send && m_hierarchy)
		m_hierarchy->SelectObject(selected, false);
}

void Viewport::UpdateCamera(Windowing::IWindow* window)
{
	if (!m_isFocus) return;

	InputSystem& input = InputSystem::Get();

	m_camera.ProcessKeyboard(input.IsKeyDown(Key::W), input.IsKeyDown(Key::S), 
		input.IsKeyDown(Key::A), input.IsKeyDown(Key::D), 
		input.IsKeyDown(Key::E), input.IsKeyDown(Key::Q), window->GetDeltaTime());
}

void Viewport::OnMouseClick(double mouseX, double mouseY)
{
	if (!m_isFocus || m_editor->GetEditorState() != EditorState::Edit) return;
	PerformPicking({ (float)mouseX, (float)mouseY });
	CommandManager::Get().StartBatch();
}

void Viewport::OnMouseRelease(double /*mouseX*/, double /*mouseY*/)
{
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
		CommandManager::Get().Execute(
			std::make_unique<TransformCommand>(m_editor->GetApp()->GetScene(), m_selectedObject, newTransform, false));
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
		CommandManager::Get().Execute(
			std::make_unique<TransformCommand>(m_editor->GetApp()->GetScene(), m_selectedObject, newTransform, false));
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
		CommandManager::Get().Execute(
			std::make_unique<TransformCommand>(m_editor->GetApp()->GetScene(), m_selectedObject, newTransform, false));
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
	m_gizmo.DrawGizmos(m_selectedObject, m_currentGizmoMode, proj * view, m_camera.GetPosition(), true, m_localGizmo);

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

			m_rhi->UnBindFrameBuffer();
			return;
		}
	}

	m_selectedAxis = Axis::None;

	RenderPassInfo picking;
	picking.m_aspectRatio = m_width / m_height;
	picking.m_camera = &m_camera;
	picking.m_flags |= UseIdAsColor;
	picking.m_flags |= IgnoreModelShader;
	picking.m_flags &= ~RenderSkybox;
	picking.m_shader = m_pickingshader;
	picking.m_target = m_fbo;

	m_pickingshader->Use();
	m_editor->GetApp()->GetScene()->Render(picking);

	SelectObject(m_editor->GetApp()->GetScene()->GetObjectAt(GetMousePixelInFbo(mousePos[0], mousePos[1])));

	m_rhi->UnBindFrameBuffer();
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

	return m_selectedObject->GetGlobalTransform().getRotation().rotate(axis);
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
	if (m_showColliders)
		pass.m_flags |= ShowCollider;

	if (m_editor->GetEditorState() == EditorState::Play)
		pass.m_camera = m_editor->GetApp()->GetScene()->GetMainCamera();
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

	m_editor->GetApp()->Render(pass);
	if (m_editor->GetEditorState() == EditorState::Edit)
	{
		if (m_showGrid) DrawGrid();
		m_rhi->ClearDepth();

		LibMath::Matrix4 view = m_camera.GetViewMatrix();
		LibMath::Matrix4 proj = m_camera.GetProjection();
		m_gizmo.DrawGizmos(m_selectedObject, m_currentGizmoMode, proj * view, m_camera.GetPosition(), false, m_localGizmo);
	}
	m_rhi->UnBindFrameBuffer();


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

	DrawUI();

	m_gui->EndPanel();
}

void Viewport::DrawUI()
{
	m_gui->SetCursorPos(m_panelPos + LibMath::Vector2(10, 35));

	if (m_gui->Button("Play")) m_editor->SetEditorState(EditorState::Play);
	m_gui->SameLine();
	if (m_gui->Button("Pause"))
	{
		if (m_editor->GetEditorState() == EditorState::Play)
			m_editor->SetEditorState(EditorState::Pause);
		else if (m_editor->GetEditorState() == EditorState::Pause)
			m_editor->SetEditorState(EditorState::Play);
	}
	m_gui->SameLine();
	if (m_gui->Button("Step") && m_editor->GetEditorState() == EditorState::Pause) m_editor->SetEditorState(EditorState::Step);
	m_gui->SameLine();
	if (m_gui->Button("Stop")) 
	{
		m_editor->SetEditorState(EditorState::Edit);
		SelectObject(nullptr);
		if (m_isFullScreen)
		{
			m_isFullScreen = false;
			m_isFirstFullScreen = true;
		}
	}
	m_gui->SameLine();
	m_gui->Checkbox("Grid", &m_showGrid);
	m_gui->SameLine();
	m_gui->Checkbox("Colliders", &m_showColliders);

	if (m_editor->GetEditorState() != EditorState::Edit) return;

	m_gui->Checkbox("Local Gizmo", &m_localGizmo);
	m_gui->SameLine();
	if (m_gui->Button("Translate")) m_currentGizmoMode = GizmoMode::Translation;
	m_gui->SameLine();
	m_gui->Checkbox("Translate Snapping", &m_isTranslationSnapping);
	m_gui->SameLine();
	m_gui->SetNextItemWidth(150.0f);
	m_gui->InputFloat("Translate value", &m_translateSnap);
	m_gui->SameLine();

	if (m_gui->Button("Rotate"))	m_currentGizmoMode = GizmoMode::Rotation;
	m_gui->SameLine();
	m_gui->Checkbox("Rotate Snapping", &m_isRotationSnapping);
	m_gui->SameLine();
	m_gui->SetNextItemWidth(150.0f);
	m_gui->InputFloat("Rotate value", &m_rotateSnap);
	m_gui->SameLine();

	if (m_gui->Button("Scale"))		m_currentGizmoMode = GizmoMode::Scale;
	m_gui->SameLine();
	m_gui->Checkbox("Scale Snapping", &m_isScaleSnapping);
	m_gui->SameLine();
	m_gui->SetNextItemWidth(150.0f);
	m_gui->InputFloat("Scale value", &m_scaleSnap);
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