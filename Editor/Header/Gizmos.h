#ifndef GIZMOS
#define GIZMOS

#include "RHI.h"
#include "Mesh.h"
#include "Shader.h"
#include "ResourceHandle.h"

namespace Apex::Data { class Object; }

namespace Apex::Editor
{
	enum class GizmoMode
	{
		Translation,
		Rotation,
		Scale
	};

	class Gizmos
	{
	public:
		Gizmos(Resources::ResourceHandle<Shader> shader, Rendering::IRHI* rhi);
		Gizmos(Gizmos const&) = delete;
		Gizmos& operator=(Gizmos const&) = delete;
		~Gizmos();


		void DrawGizmos(Data::Object* object, GizmoMode mode, LibMath::Matrix4 viewProj, LibMath::Vector3 viewPos);

		bool IsOnGizmo(LibMath::Vector2 pointOnScreen);

	private:

		void DrawTranslation(Data::Object* object, float distance);
		void DrawRotation(Data::Object* object, float distance);
		void DrawScale(Data::Object* object, float distance);

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
	};
}


#endif // !GIZMOS
