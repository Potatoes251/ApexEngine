#include "Viewport.h"

using namespace Apex::Data;

using namespace Apex::Editor;
using namespace Apex::Rendering;

Viewport::Viewport(Resources::ResourceHandle<Shader> shader, Rendering::Scene* scene, Rendering::IRHI* rhi, UserInterface::IGUI* gui)
	: m_gizmo(shader, rhi), m_currentScene(scene), m_rhi(rhi), m_gui(gui)
{
	m_fbo = m_rhi->CreateFrameBuffer(m_width, m_height, m_texture);
    m_selectedObject = m_currentScene->GetObjectAt(0);
}

void Viewport::Update()
{

}

void Viewport::Draw()
{
	LibMath::Matrix4 view = m_camera.GetViewMatrix();
	LibMath::Matrix4 proj = m_camera.GetProjectionMatrix(m_width / m_height);

	m_rhi->BindFrameBuffer(m_fbo, m_width, m_height);
	m_rhi->Clear();

	m_currentScene->Render(view, proj, m_camera.GetPosition());
    m_rhi->SetDepthTest(false);
	m_gizmo.DrawGizmos(m_selectedObject, m_currentGizmoMode, proj * view, m_camera.GetPosition());
    m_rhi->SetDepthTest(true);
    m_rhi->UnBindFrameBuffer();

	m_gui->BeginPanel("Viewport");
	LibMath::Vector2 pos = m_gui->GetPanelPos() + (m_gui->GetPanelSize() * 0.5f);

	m_gui->DrawImage2D(m_rhi->GetTexture(m_texture), m_gui->GetPanelPos(), m_gui->GetPanelPos() + m_gui->GetPanelSize());

	m_gui->EndPanel();
}