#include "Viewport.h"

#include <LibMath/Arithmetic.h>
#include <LibMath/Colision3D.h>


#include "Editor.h"
#include "Scene.h"
#include "RHI.h"
#include "UI.h"

using namespace Apex::Data;

using namespace Apex::Editor;
using namespace Apex::Rendering;

Viewport::Viewport(Resources::ResourceHandle<Shader> pickingShader, Resources::ResourceHandle<Shader> gridShader, 
	Rendering::Scene* scene, Rendering::IRHI* rhi, UserInterface::IGUI* gui, Editor* editor)
	: m_gizmo(pickingShader, rhi), m_gridShader(gridShader), m_currentScene(scene), m_rhi(rhi), m_gui(gui) 
{
	m_fbo = m_rhi->CreateFrameBuffer(m_width, m_height, m_textureFbo);
	m_pickingshader = pickingShader;
	m_editor = editor;

	CreateGrid();
}

void Viewport::OnMouseClick(double mouseX, double mouseY)
{
	if (m_editor->GetEditorState() != EditorState::Edit) return;
	PerformPicking({ (float)mouseX, (float)mouseY });
}

void Viewport::OnMouseRelease(double /*mouseX*/, double /*mouseY*/)
{
	if (m_editor->GetEditorState() != EditorState::Edit) return;
	m_selectedAxis = Axis::None;
}

void Viewport::OnMouseMove(double mouseX, double mouseY)
{
	if (m_selectedAxis == Axis::None || !m_selectedObject)
		return;

	if (m_currentGizmoMode != GizmoMode::Rotation) ComputePlane();
	else ComputePlaneRotation();

	const LibMath::Vector3 axis = GetAxis();

	LibMath::Matrix4 viewProj = m_camera.GetProjectionMatrix(m_width / m_height) *m_camera.GetViewMatrix();

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
	else if (IsAxisPlane())
	{
		movementVec = delta;
	}
	else if (IsAxisFree())
	{
		movementVec = delta;
	}

	switch (m_currentGizmoMode)
	{
	case GizmoMode::Translation:
	{
		if (m_isTranslationSnapping) Snap(movementVec);
		m_selectedObject->GetLocalTransform().setPosition(m_initialTransform.getPosition() + movementVec);
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

		m_selectedObject->GetLocalTransform().setRotation(deltaAngle * m_initialTransform.getRotation());
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
		m_selectedObject->GetLocalTransform().setScale(newScale);
		break;
	}
	default:
		assert(false && "Unhandled gizmo mode");
		break;
	}
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

bool Apex::Editor::Viewport::IsOnGizmo(double mouseX, double mouseY)
{
	if (!m_selectedObject) return false;

	LibMath::Vector3 center = m_selectedObject->GetGlobalTransform().getPosition();
	LibMath::Matrix4 viewProj = m_camera.GetProjectionMatrix(m_width / m_height) * m_camera.GetViewMatrix();
	m_selectedAxis = m_gizmo.IsOnGizmo({ (float)mouseX, (float)mouseY }, m_currentGizmoMode, center, viewProj, m_panelPos, m_panelSize);

	if (m_selectedAxis == Axis::None) return false;

	m_initialTransform = m_selectedObject->GetGlobalTransform();

	LibMath::Line3D ray = ScreenToWorldRay({ (float)mouseX ,(float)mouseY }, viewProj, m_panelPos, m_panelSize);
	if (!LibMath::intersectionRayPlane(ray, m_axisPlane, m_startingPoint)) return false;
	
	if (m_currentGizmoMode != GizmoMode::Rotation)
	{
		m_startingPoint = center + GetAxis() * ((m_startingPoint - center).dot(GetAxis()));
	}
	return true;
}

void Viewport::PerformPicking(LibMath::Vector2 mousePos)
{
	if (!m_pickingshader.IsReady()) return;

	if (IsOnGizmo(mousePos[0], mousePos[1])) return;

	m_rhi->BindFrameBuffer(m_fbo, m_width, m_height);
	m_rhi->Clear();

	RenderPassInfo picking;
	picking.m_aspectRatio = m_width / m_height;
	picking.m_camera = &m_camera;
	picking.m_useIdAsColor = true;
	picking.m_useModelShader = false;
	picking.m_shader = m_pickingshader;

	m_pickingshader->Use();
	m_currentScene->Render(picking);

	m_selectedObject = m_currentScene->GetObjectAt(GetMousePixelInFbo(mousePos[0], mousePos[1]));

	m_rhi->UnBindFrameBuffer();
}

LibMath::Vector3 Viewport::GetAxis()
{
	switch (m_selectedAxis)
	{
	case Axis::X:		return { 1, 0, 0 };
	case Axis::Y:		return { 0, 1, 0 };
	case Axis::Z:		return { 0, 0, 1 };
	case Axis::None:	return { 0, 0, 0 };
	default:			return { 0, 0, 0 };
	}
}

LibMath::Vector3 Apex::Editor::Viewport::GetAxisPlane()
{
	switch (m_selectedAxis)
	{
	case Axis::XY:		return { 0, 0, 1 };
	case Axis::XZ:		return { 0, 1, 0 };
	case Axis::YZ:		return { 1, 0, 0 };
	case Axis::XYZ:		return m_camera.GetFront();
	case Axis::None:	return { 0, 0, 0 };
	default:			return { 0, 0, 0 };
	}
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

	LibMath::Matrix4 view = m_camera.GetViewMatrix();
	LibMath::Matrix4 proj = m_camera.GetProjectionMatrix(aspectRatio);

	m_rhi->BindFrameBuffer(m_fbo, m_width, m_height);
	m_rhi->Clear();

	RenderPassInfo pass;
	pass.m_aspectRatio = aspectRatio;
	pass.m_camera = &m_camera;

	m_gui->BeginPanel("Viewport", nullptr, false);

	m_panelPos = m_gui->GetPanelPos();
	m_panelSize = m_gui->GetPanelSize();

	m_editor->GetApp()->Render(&m_camera);
	if (m_editor->GetEditorState() == EditorState::Edit)
	{
		DrawGrid();
		m_rhi->ClearDepth();
		m_gizmo.DrawGizmos(m_selectedObject, m_currentGizmoMode, proj * view, m_camera.GetPosition());
	}
	m_rhi->UnBindFrameBuffer();


	m_gui->DrawImageBackground(m_rhi->GetTexture(m_textureFbo), m_gui->GetPanelPos(), m_gui->GetPanelPos() + m_gui->GetPanelSize());

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
	if (m_gui->Button("Stop")) m_editor->SetEditorState(EditorState::Edit);

	if (m_editor->GetEditorState() != EditorState::Edit) return;

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
	LibMath::Matrix4 proj = m_camera.GetProjectionMatrix(m_width / m_height);
	LibMath::Matrix4 invViewProj = proj * view;
	m_gridShader->SetUniform("uViewProj", invViewProj);
	invViewProj.inverse();
	m_gridShader->SetUniform("uInvViewProj", invViewProj);

	m_gridShader->SetUniform("uCameraPos", m_camera.GetPosition());
	m_grid.Draw();
}

void Viewport::CreateGrid()
{
	m_grid.m_vertices.push_back(Vertex());

	m_grid.m_indices =
	{
		0, 1, 2, // Triangle 1
		0, 2, 3  // Triangle 2
	};

	m_grid.SetRhi(m_rhi);
	m_grid.UploadToGpu();
	m_grid.ClearVertices();
}
