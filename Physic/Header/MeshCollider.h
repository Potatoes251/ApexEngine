#ifndef MESH_COLLIDER
#define MESH_COLLIDER

#include "Collider.h"
#include "Mesh.h"
#include "ResourceHandle.h"

namespace Apex::Physic
{
	enum class MeshColliderType
	{
		Triangle,
		Convex
	};

	class MeshCollider : public Collider
	{
	public:
		MeshCollider(Resources::ResourceHandle<Mesh> mesh, MeshColliderType type, PhysMatHandle material = PhysMatHandle()) 
			: m_mesh(mesh), m_type(type), Collider(material) {}
		MeshCollider(MeshCollider const&);
		MeshCollider& operator=(MeshCollider const&);

		~MeshCollider() = default;

		ColliderType GetType() const override { return ColliderType::Mesh; }
		MeshColliderType GetMeshColliderType() const { return m_type; }

		Resources::ResourceHandle<Mesh> GetMesh() const { return m_mesh; }

		Resources::ResourceHandle<Mesh> SetMeshColliderType(MeshColliderType type) { m_type = type; }

	private:
		Resources::ResourceHandle<Mesh> m_mesh;
		MeshColliderType m_type;
	};
}

#endif // !MESH_COLLIDER

