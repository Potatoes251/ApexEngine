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
	m_rhi->BindFrameBuffer(m_fbo, m_width, m_height);
	m_currentScene->Render(m_camera.GetViewMatrix(), m_camera.GetProjectionMatrix(m_width / m_height), m_camera.GetPosition());
	m_gizmo.DrawGizmos(m_selectedObject, m_currentGizmoMode);

	m_rhi->UnBindFrameBuffer();
}