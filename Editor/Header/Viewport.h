#ifndef VIEWPORT
#define VIEWPORT


#include "Shader.h"
#include "ResourceHandle.h"

#include "Gizmos.h"

#include "Scene.h"
#include "Camera.h"
#include "RHI.h"
#include "UI.h"

namespace Apex::Data { class Object; }

namespace Apex::Editor
{
	class Viewport
	{
	public:
		Viewport(Resources::ResourceHandle<Shader> shader, Rendering::Scene* scene, Rendering::IRHI* rhi, UserInterface::IGUI* gui);
		Viewport(Viewport const&) = delete;
		Viewport& operator=(Viewport const&) = delete;

		Rendering::Camera&  GetCamera() { return m_camera; }
		void				Update();
		void				OnMouseClick(int mouseX, int mouseY);
		void				Draw();

	private:
		int					GetMousePixelInFbo(int mouseX, int mouseY);

		Gizmos m_gizmo;
		GizmoMode m_currentGizmoMode = GizmoMode::Rotation;
		
		Rendering::Camera m_camera;

		Apex::UserInterface::IGUI* m_gui = nullptr;
		
		Data::Object* m_selectedObject = nullptr;
		Rendering::Scene* m_currentScene = nullptr;
		Rendering::IRHI* m_rhi = nullptr;

		LibMath::Vector2 m_panelPos;
		LibMath::Vector2 m_panelSize;

		Resources::ResourceHandle<Shader> m_shader;

		Rendering::RHIFrameBufferHandle m_fbo;
		Rendering::RHITextureHandle m_textureFbo;

		float m_width = 1280;
		float m_height = 720;

		bool m_isHovered = false;
	};
}


#endif // !VIEWPORT
