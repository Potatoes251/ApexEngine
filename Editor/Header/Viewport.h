#ifndef VIEWPORT
#define VIEWPORT


#include "LibMath/Geometry3D.h"
#include "LibMath/Transform.h"

#include "Shader.h"
#include "ResourceHandle.h"

#include "Gizmos.h"

#include "Camera.h"

namespace Apex::Data { class Object; }

namespace Apex::UserInterface { class IGUI; }
namespace Apex::Rendering { class IRHI; }
namespace Apex::Rendering { class Scene; }

namespace Apex::Editor
{
	class Viewport
	{
	public:
		Viewport(Resources::ResourceHandle<Shader> shader, Rendering::Scene* scene, Rendering::IRHI* rhi, UserInterface::IGUI* gui);
		Viewport(Viewport const&) = delete;
		Viewport& operator=(Viewport const&) = delete;

		Rendering::Camera&  GetCamera() { return m_camera; }
		void				ChangeScene(Rendering::Scene* newScene) { m_currentScene = newScene; }
		void				OnMouseClick(double mouseX, double mouseY);
		void				OnMouseRelease(double /*mouseX*/, double /*mouseY*/);
		void				OnMouseMove(double mouseX, double mouseY);
		void				Draw();

	private:
		int					GetMousePixelInFbo(float mouseX, float mouseY);

		bool				IsOnGizmo(double mouseX, double mouseY);

		LibMath::Vector3	GetAxis();
		bool				IsSingleAxis() const;
		bool				IsPlaneAxis() const;
		void				ComputePlane();
		void				ComputePlaneRotation();
		float				Snap(float rotationAngle = 0) const;

		Gizmos		m_gizmo;
		GizmoMode	m_currentGizmoMode = GizmoMode::Scale;
		Axis		m_selectedAxis = Axis::None;
		

		Resources::ResourceHandle<Shader>	m_shader;
		Rendering::RHIFrameBufferHandle		m_fbo;
		Rendering::RHITextureHandle			m_textureFbo;


		Rendering::Camera		m_camera;

		UserInterface::IGUI*	m_gui = nullptr;
		Rendering::IRHI*		m_rhi = nullptr;
		Rendering::Scene*		m_currentScene = nullptr;
		Data::Object*			m_selectedObject = nullptr;

		LibMath::Vector2		m_panelPos;
		LibMath::Vector2		m_panelSize;

		// used for changing the object's transform with gizmos
		LibMath::Transform		m_initialTransform;
		LibMath::Plane			m_axisPlane;
		LibMath::Vector3		m_startingPoint;


		float	m_width = 1280;
		float	m_height = 720;

		float	m_translateSnap = 1.f;
		float	m_rotateSnap = .2f;
		float	m_scaleSnap = 1.f;

		bool	m_isHovered = false;
		bool	m_isTranslationSnapping = true;
		bool	m_isRotationSnapping = true;
		bool	m_isScaleSnapping = true;
	};
}


#endif // !VIEWPORT
