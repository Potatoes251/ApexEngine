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
		// DO NOT CHANGE ORDER
		X, Y, Z,
		XY, XZ, YZ,
		XYZ,
		None,
	};

	class Gizmos
	{
	public:
		Gizmos(Resources::ResourceHandle<Shader> shader, Rendering::IRHI* rhi);
		Gizmos(Gizmos const&) = delete;
		Gizmos& operator=(Gizmos const&) = delete;
		~Gizmos() = default;

		void DrawGizmos(Data::Object* object, GizmoMode mode, LibMath::Matrix4 viewProj, LibMath::Vector3 viewPos, bool picking, bool local);
	private:

		void DrawTranslation(Data::Object* object, bool picking, bool local);
		void DrawTranslationPlanes(LibMath::Matrix4 transMatrix, bool picking);
		void DrawTranslationCenter(LibMath::Matrix4 transMatrix, bool picking);
		void DrawTranslationArrows(LibMath::Matrix4 transMatrix, bool picking);

		void DrawRotation(Data::Object* object, bool picking, bool local);
		void DrawScale(Data::Object* object, bool picking);

		LibMath::Vector3 GetColor(Axis axis, bool picking = false);

		// load basic shape
		void CreateCube();
		void CreateCone();
		void CreateCylinder();
		void CreateCircle();
		void CreateThorus();

		Mesh m_cube;
		Mesh m_cone;
		Mesh m_cylinder;
		Mesh m_circle;
		Mesh m_thorus;
		
		Resources::ResourceHandle<Shader> m_shader;

		float m_distanceToCamera = 0.f;
	};

	LibMath::Vector2	GetScreenPos(
		LibMath::Vector3 worldPos, LibMath::Matrix4 const& viewProj, LibMath::Vector2 panelOffset, LibMath::Vector2 panelSize);
	LibMath::Line3D		ScreenToWorldRay(
		LibMath::Vector2 mousePosOnScreen, LibMath::Matrix4 viewProj, LibMath::Vector2 panelOffset, LibMath::Vector2 panelSize);
}


#endif // !GIZMOS
