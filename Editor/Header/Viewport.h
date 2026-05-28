#ifndef VIEWPORT
#define VIEWPORT


#include "LibMath/Geometry3D.h"
#include "LibMath/Transform.h"

#include "Shader.h"
#include "ResourceHandle.h"

#include "Gizmos.h"

#include "FirstPersonCamera.h"

namespace Apex::Data { class Object; }

namespace Apex::UserInterface { class IGUI; }
namespace Apex::Windowing { class IWindow; }
namespace Apex::Rendering 
{ 
	class IRHI; 
	class Scene;
}

namespace Apex::Editor
{
	struct BuildSceneEntry
	{
		std::string path;
		bool        selected = true;
	};

	class Editor;
	class Hierarchy;

	class Viewport
	{
	public:
		Viewport(Resources::ResourceHandle<Shader> pickingShader, Resources::ResourceHandle<Shader> gridShader, 
			Rendering::IRHI* rhi, UserInterface::IGUI* gui, Editor* editor);
		Viewport(Viewport const&) = delete;
		Viewport& operator=(Viewport const&) = delete;
		~Viewport();

		Rendering::FirstPersonCamera&  GetCamera() { return m_camera; }
		void				SetHierarchy(Hierarchy* hierarchy) { m_hierarchy = hierarchy; }
		void				SelectObject(Data::Object* selected, bool send = true);

		void				UpdateCamera(Windowing::IWindow* window);
		void				OnMouseClick(double mouseX, double mouseY);
		void				OnMouseRelease(double /*mouseX*/, double /*mouseY*/);
		void				OnMouseMove(double mouseX, double mouseY);
		void				ToggleFullScreen();

		void				Draw();

		bool				IsFocus() const { return m_isFocus; }

	private:
		int					GetMousePixelInFbo(float mouseX, float mouseY);

		void				PerformPicking(LibMath::Vector2 mousePos);

		LibMath::Vector3	GetAxis();
		LibMath::Vector3	GetAxisPlane();
		bool				IsAxisSingle() const;
		bool				IsAxisPlane() const;
		bool				IsAxisFree() const;
		void				ComputePlane();
		void				ComputePlaneRotation();
		void				Snap(float& value, float snapValue) const;
		void				Snap(LibMath::Vector3& value) const;

		void				DrawUI();
		void				DrawGrid();
		void				DrawBuildPopup();

		void				DrawPlayBar();
		void				DrawRightToolbar();
		void				DrawCameraSpeed();

		void				DrawSnapButtons(const float button, const float pad, float rightX, float& curY, const float round);

		void				DrawPillGroup(LibMath::Vector2 topLeft, LibMath::Vector2 size, float rounding) const;
		bool				IconButton(const char* iconPath, const char* fallbackLabel, LibMath::Vector2 size, 
										bool active = false, Apex::UserInterface::Color tint = Apex::UserInterface::Color(1.0f, 1.0f, 1.0f, 1.0f));

		Gizmos		m_gizmo;
		GizmoMode	m_currentGizmoMode = GizmoMode::Translation;
		Axis		m_selectedAxis = Axis::None;
		
		Resources::ResourceHandle<Shader>	m_gridShader;

		Resources::ResourceHandle<Shader>	m_pickingshader;
		Rendering::RHIFrameBufferHandle		m_fbo;
		Rendering::RHITextureHandle			m_textureFbo;


		Rendering::FirstPersonCamera		m_camera;

		UserInterface::IGUI*	m_gui = nullptr;
		Rendering::IRHI*		m_rhi = nullptr;
		Data::Object*			m_selectedObject = nullptr;
		Editor*					m_editor = nullptr;
		Hierarchy*				m_hierarchy = nullptr;

		LibMath::Vector2		m_panelPos;
		LibMath::Vector2		m_panelSize;
		LibMath::Vector2		m_oldPanelPos;
		LibMath::Vector2		m_oldPanelSize;

		// used for changing the object's transform with gizmos
		LibMath::Transform		m_initialTransform;
		LibMath::Plane			m_axisPlane;
		LibMath::Vector3		m_startingPoint;

		std::vector<BuildSceneEntry>  m_buildScenes;

		float	m_width = 1280;
		float	m_height = 720;

		float	m_translateSnap = 1.f;
		float	m_rotateSnap = 2.f;
		float	m_scaleSnap = 1.f;

		bool	m_isFocus = false;
		bool	m_localGizmo = true;
		bool	m_isTranslationSnapping = false;
		bool	m_isRotationSnapping = false;
		bool	m_isScaleSnapping = false;
		bool	m_isFullScreen = false;
		bool	m_isFirstFullScreen = false;
		bool	m_showGrid = false;
		bool	m_showColliders = false;

		bool    m_showBuildPopup = false;


	};
}


#endif // !VIEWPORT
