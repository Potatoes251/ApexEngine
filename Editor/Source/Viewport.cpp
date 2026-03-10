#include "Viewport.h"

using namespace Apex::Data;

using namespace Apex::Editor;
using namespace Apex::Rendering;


Viewport::Viewport(Resources::ResourceHandle<Shader> shader, Rendering::Scene* scene, Rendering::IRHI* rhi, UserInterface::IGUI* gui)
	: m_gizmo(shader, rhi), m_currentScene(scene), m_rhi(rhi), m_gui(gui)
{
	m_fbo = m_rhi->CreateFrameBuffer(m_width, m_height, m_texture);
}

void Viewport::Update()
{
	
}

void Viewport::Draw()
{
	LibMath::Matrix4 view = m_camera.GetViewMatrix();
	LibMath::Matrix4 proj = m_camera.GetProjectionMatrix(m_width / m_height);

	m_rhi->BindFrameBuffer(m_fbo, m_width, m_height);
	m_gizmo.DrawGizmos(m_selectedObject, m_currentGizmoMode, proj * view);
	m_currentScene->Render(view, proj, m_camera.GetPosition());

	m_rhi->UnBindFrameBuffer();
}