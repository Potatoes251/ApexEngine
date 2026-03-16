#include "Viewport.h"

#include <LibMath/Arithmetic.h>
#include <LibMath/Colision3D.h>

#include "Scene.h"
#include "RHI.h"
#include "UI.h"

using namespace Apex::Data;

using namespace Apex::Editor;
using namespace Apex::Rendering;

Viewport::Viewport(Resources::ResourceHandle<Shader> shader, Rendering::Scene* scene, Rendering::IRHI* rhi, UserInterface::IGUI* gui)
	: m_gizmo(shader, rhi), m_currentScene(scene), m_rhi(rhi), m_gui(gui)
{
	m_fbo = m_rhi->CreateFrameBuffer(m_width, m_height, m_textureFbo);
	m_shader = shader;
}

void Viewport::OnMouseClick(double mouseX, double mouseY)
{
	if (!m_isHovered || !m_shader.IsReady()) return;

	if (IsOnGizmo(mouseX, mouseY)) return;

	m_rhi->BindFrameBuffer(m_fbo, m_width, m_height);
	m_rhi->Clear();

	RenderPassInfo picking;
	picking.m_aspectRatio = m_width / m_height;
	picking.m_camera = &m_camera;
	picking.m_useIdAsColor = true;
	picking.m_useModelShader = false;
	picking.m_shader = m_shader;

	m_shader->Use();
	m_currentScene->Render(picking);

	m_selectedObject = m_currentScene->GetObjectAt(GetMousePixelInFbo((float)mouseX, (float)mouseY));

	m_rhi->UnBindFrameBuffer();
}

void Viewport::OnMouseRelease(double /*mouseX*/, double /*mouseY*/)
{
	m_selectedAxis = Axis::None;
}

void Viewport::OnMouseMove(double mouseX, double mouseY)
{
	if (m_selectedAxis == Axis::None || !m_selectedObject) return;

	LibMath::Matrix4 viewProj = m_camera.GetProjectionMatrix(m_width / m_height) * m_camera.GetViewMatrix();
	LibMath::Line3D ray = ScreenToWorldRay({ (float)mouseX ,(float)mouseY }, viewProj, m_panelPos, m_panelSize);

	LibMath::Vector3 hit;

	if (!LibMath::intersectionRayPlane(ray, m_axisPlane, hit)) return;

	LibMath::Vector3 delta = hit - m_startingPoint;

	float movement = delta.dot(GetAxis());

	LibMath::Vector3 center = m_initialTransform.getPosition();

	LibMath::Vector3 startVec = (m_startingPoint - center).normalize();
	LibMath::Vector3 currentVec = (hit - center).normalize();

	float angle = atan2(
		GetAxis().dot(startVec.cross(currentVec)), // signed rotation around axis
		startVec.dot(currentVec)               // cosine of angle
	);

	switch (m_currentGizmoMode)
	{
	case Apex::Editor::GizmoMode::Translation:
		m_selectedObject->GetGlobalTransform().setPosition(m_initialTransform.getPosition() + GetAxis() * movement);
		break;
	case Apex::Editor::GizmoMode::Rotation:
	{
		LibMath::Quaternion delta = LibMath::Quaternion(LibMath::Radian(angle), GetAxis());
		m_selectedObject->GetGlobalTransform().setRotation(delta * m_initialTransform.getRotation());
	}		break;
	case Apex::Editor::GizmoMode::Scale:
		m_selectedObject->GetGlobalTransform().setScale(m_initialTransform.getScale() + GetAxis() * movement);
		break;
	default:
		assert(false && "Unhandled gizmo mode");
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

	m_gui->BeginPanel("Viewport");

	m_panelPos = m_gui->GetPanelPos();
	m_panelSize = m_gui->GetPanelSize();
	m_isHovered = m_gui->IsWindowHovered();

	m_currentScene->Render(pass);
    m_rhi->SetDepthTest(false);
	m_gizmo.DrawGizmos(m_selectedObject, m_currentGizmoMode, proj * view, m_camera.GetPosition());
    m_rhi->SetDepthTest(true);
    m_rhi->UnBindFrameBuffer();

	m_gui->DrawImage2D(m_rhi->GetTexture(m_textureFbo), m_gui->GetPanelPos(), m_gui->GetPanelPos() + m_gui->GetPanelSize());

	m_gui->EndPanel();
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

	if (m_currentGizmoMode != GizmoMode::Rotation) ComputePlane();
	else ComputePlaneRotation();

	LibMath::Line3D ray = ScreenToWorldRay({ (float)mouseX ,(float)mouseY }, viewProj, m_panelPos, m_panelSize);
	if (!LibMath::intersectionRayPlane(ray, m_axisPlane, m_startingPoint)) return false;
	
	if (m_currentGizmoMode != GizmoMode::Rotation)
	{
		m_startingPoint = center + GetAxis() * ((m_startingPoint - center).dot(GetAxis()));
	}
	
	return true;
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

void Viewport::ComputePlane()
{
	if (m_selectedAxis == Axis::None || !m_selectedObject) return;

	LibMath::Vector3 axis = GetAxis();

	LibMath::Vector3 planeNormal = m_camera.GetFront() - axis * m_camera.GetFront().dot(axis);

	if (planeNormal.magnitudeSquared() < g_epsilon)
		planeNormal = axis.cross(m_camera.GetRight());

	m_axisPlane = LibMath::Plane(planeNormal, -planeNormal.dot(m_selectedObject->GetGlobalTransform().getPosition()));
}

void Viewport::ComputePlaneRotation()
{
	if (m_selectedAxis == Axis::None || !m_selectedObject) return;

	LibMath::Vector3 planeNormal = GetAxis();

	m_axisPlane = LibMath::Plane(planeNormal, -planeNormal.dot(m_selectedObject->GetGlobalTransform().getPosition()));
}
