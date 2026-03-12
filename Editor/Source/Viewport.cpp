#include "Viewport.h"
#include <LibMath/Arithmetic.h>

using namespace Apex::Data;

using namespace Apex::Editor;
using namespace Apex::Rendering;

Viewport::Viewport(Resources::ResourceHandle<Shader> shader, Rendering::Scene* scene, Rendering::IRHI* rhi, UserInterface::IGUI* gui)
	: m_gizmo(shader, rhi), m_currentScene(scene), m_rhi(rhi), m_gui(gui)
{
	m_fbo = m_rhi->CreateFrameBuffer(m_width, m_height, m_textureFbo);
	m_shader = shader;
}

void Viewport::Update()
{

}

void Viewport::OnMouseClick(int mouseX, int mouseY)
{
	if (!m_isHovered || !m_shader.IsReady()) return;

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

	int id = GetMousePixelInFbo(mouseX, mouseY);

	printf("id is %d\n", id);
	m_selectedObject = m_currentScene->GetObjectAt(id);

	m_rhi->UnBindFrameBuffer();
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

int Viewport::GetMousePixelInFbo(int mouseX, int mouseY)
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

	return m_rhi->GetPixelData(fboX, fboY);
}
