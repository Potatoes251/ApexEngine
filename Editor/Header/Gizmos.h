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
		Gizmos(Resources::ResourceHandle<Shader> shader);

		void DrawGizmos(Data::Object* object, GizmoMode mode);

	private:

		void DrawTranslation(Data::Object* object);
		void DrawRotation(Data::Object* object);
		void DrawScale(Data::Object* object);

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
