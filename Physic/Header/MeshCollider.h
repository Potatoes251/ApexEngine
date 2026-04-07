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
		MeshCollider(Resources::ResourceHandle<Mesh> mesh, MeshColliderType type, bool isTrigger, PhysMatHandle material = PhysMatHandle())
			: m_mesh(mesh), m_type(type), Collider(material, isTrigger) {}
		MeshCollider(MeshCollider const&);
		MeshCollider& operator=(MeshCollider const&);

		~MeshCollider() = default;

		std::unique_ptr<Component> Clone() override { return std::make_unique<MeshCollider>(*this); }

		ColliderType GetType() const override { return ColliderType::Mesh; }
		MeshColliderType GetMeshColliderType() const { return m_type; }
		const char* GetTypeName() const override { return "MeshCollider"; };
		std::vector<ExposedVar> GetExposedVariables() override;

		Resources::ResourceHandle<Mesh> GetMesh() const { return m_mesh; }

		void SetMeshColliderType(MeshColliderType type) { m_type = type; }

		void Serialize(std::ostream& out) const override;

	private:
		Resources::ResourceHandle<Mesh> m_mesh;
		MeshColliderType m_type;
	};
}

#endif // !MESH_COLLIDER