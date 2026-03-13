#ifndef GIZMOS
#define GIZMOS

#include "RHI.h"
#include "Mesh.h"
#include "Shader.h"
#include "ResourceHandle.h"

namespace LibMath { class Line3D; }
namespace Apex::Data { class Object; }

namespace Apex::Editor
{
	enum class GizmoMode
	{
		Translation,
		Rotation,
		Scale
	};

	enum class Axis
	{
		X,
		Y,
		Z,
		None
	};

	class Gizmos
	{
	public:
		Gizmos(Resources::ResourceHandle<Shader> shader, Rendering::IRHI* rhi);
		Gizmos(Gizmos const&) = delete;
		Gizmos& operator=(Gizmos const&) = delete;
		~Gizmos();


		void DrawGizmos(Data::Object* object, GizmoMode mode, LibMath::Matrix4 viewProj, LibMath::Vector3 viewPos);

		Axis IsOnGizmo(
			LibMath::Vector2 mousePosOnScreen, GizmoMode mode, LibMath::Vector3 worldPos, 
			LibMath::Matrix4 const& viewProj, LibMath::Vector2 panelOffset, LibMath::Vector2 panelSize, float distanceToCamera);

	private:

		void DrawTranslation(Data::Object* object);
		void DrawRotation(Data::Object* object);
		void DrawScale(Data::Object* object);

		Axis IsOnTranslation(LibMath::Vector2 mousePosOnScreen, LibMath::Vector3 worldPos, LibMath::Matrix4 const& viewProj, LibMath::Vector2 panelOffset, LibMath::Vector2 panelSize);
		Axis IsOnRotation(LibMath::Vector2 mousePosOnScreen, LibMath::Vector3 worldPos, LibMath::Matrix4 const& viewProj, LibMath::Vector2 panelOffset, LibMath::Vector2 panelSize);
		Axis IsOnScale(LibMath::Vector2 mousePosOnScreen, LibMath::Vector3 worldPos, LibMath::Matrix4 const& viewProj, LibMath::Vector2 panelOffset, LibMath::Vector2 panelSize);

		// load basic shape
		void CreateCube();
		void CreateCone();
		void CreateCylinder();
		void CreateCircle();

		LibMath::Vector2	GetScreenPos(LibMath::Vector3 worldPos, LibMath::Matrix4 const& viewProj, LibMath::Vector2 panelOffset, LibMath::Vector2 panelSize);
		LibMath::Line3D		ScreenToWorldRay(LibMath::Vector2 mousePosOnScreen, LibMath::Matrix4 viewProj, LibMath::Vector2 panelOffset, LibMath::Vector2 panelSize);

		Mesh m_cube;
		Mesh m_cone;
		Mesh m_cylinder;
		Mesh m_circle;
		
		float m_distanceToCamera = 0.f;

		Resources::ResourceHandle<Shader> m_shader;
	};
}


#endif // !GIZMOS
