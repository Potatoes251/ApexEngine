#ifndef MESH_COLLIDER
#define MESH_COLLIDER

#include "Collider.h"
#include "Mesh.h"
#include "ResourceHandle.h"

namespace Apex::Physic
{
	class MeshCollider : public Collider
	{
	public:
		MeshCollider(LibMath::Vector3 halfExtents) : m_halfExtents(halfExtents) {}
		MeshCollider(MeshCollider const&);
		MeshCollider& operator=(MeshCollider const&);

		~MeshCollider() = default;

		ColliderType GetType() const override { return ColliderType::Mesh; }

		LibMath::Vector3 GetHalfExtents() const { return m_halfExtents; }

	private:
		LibMath::Vector3 m_halfExtents;
	};
}

#endif // !MESH_COLLIDER

