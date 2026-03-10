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
		void				Draw();

	private:

		Gizmos m_gizmo;
		GizmoMode m_currentGizmoMode = GizmoMode::Translation;
		
		Rendering::Camera m_camera;

		Apex::UserInterface::IGUI* m_gui = nullptr;
		
		Data::Object* m_selectedObject = nullptr;
		Rendering::Scene* m_currentScene = nullptr;
		Rendering::IRHI* m_rhi = nullptr;

		Rendering::RHIFrameBufferHandle m_fbo;
		Rendering::RHITextureHandle m_texture;

		float m_width = 1280;
		float m_height = 720;
	};
}


#endif // !VIEWPORT
