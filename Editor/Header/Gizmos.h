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
		None,
		X, Y, Z,
		XY, XZ, YZ,
		XYZ,
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
			LibMath::Matrix4 const& viewProj, LibMath::Vector2 panelOffset, LibMath::Vector2 panelSize) const;

	private:

		void DrawTranslation(Data::Object* object);
		void DrawRotation(Data::Object* object);
		void DrawScale(Data::Object* object);

		Axis IsOnTranslation(
			LibMath::Vector2 mousePosOnScreen, LibMath::Vector3 worldPos, LibMath::Matrix4 const& viewProj, 
			LibMath::Vector2 panelOffset, LibMath::Vector2 panelSize) const;
		Axis IsOnRotation(
			LibMath::Vector2 mousePosOnScreen, LibMath::Vector3 worldPos, LibMath::Matrix4 const& viewProj,
			LibMath::Vector2 panelOffset, LibMath::Vector2 panelSize) const;
		Axis IsOnScale(
			LibMath::Vector2 mousePosOnScreen, LibMath::Vector3 worldPos, LibMath::Matrix4 const& viewProj, 
			LibMath::Vector2 panelOffset, LibMath::Vector2 panelSize) const;

		// load basic shape
		void CreateCube();
		void CreateCone();
		void CreateCylinder();
		void CreateCircle();

		Mesh m_cube;
		Mesh m_cone;
		Mesh m_cylinder;
		Mesh m_circle;
		
		Resources::ResourceHandle<Shader> m_shader;

		float m_distanceToCamera = 0.f;
	};

	LibMath::Vector2	GetScreenPos(LibMath::Vector3 worldPos, LibMath::Matrix4 const& viewProj, LibMath::Vector2 panelOffset, LibMath::Vector2 panelSize);
	LibMath::Line3D		ScreenToWorldRay(LibMath::Vector2 mousePosOnScreen, LibMath::Matrix4 viewProj, LibMath::Vector2 panelOffset, LibMath::Vector2 panelSize);
}


#endif // !GIZMOS
