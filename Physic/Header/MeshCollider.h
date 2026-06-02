#ifndef MESH_COLLIDER
#define MESH_COLLIDER

#include "Collider.h"
#include "ResourceHandle.h"

class Model;

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
		MeshCollider(Resources::ResourceHandle<Model> model, MeshColliderType type, bool isTrigger)
			: m_model(model), m_type(type), Collider(isTrigger) {}
		MeshCollider(MeshCollider const&) = default;
		MeshCollider& operator=(MeshCollider const&) = default;

		~MeshCollider() = default;

		std::unique_ptr<Component> Clone() override { return std::make_unique<MeshCollider>(*this); }

		ColliderType GetType() const override { return ColliderType::Mesh; }
		MeshColliderType GetMeshColliderType() const { return m_type; }
		const char* GetTypeName() const override { return "MeshCollider"; }
		std::vector<ExposedVar> GetExposedVariables() override;

		Resources::ResourceHandle<Model> GetModel() const { return m_model; }

		void SetMeshColliderType(MeshColliderType type) { m_type = type; }

		void Serialize(std::ostream& out) const override;

	private:
		Resources::ResourceHandle<Model> m_model;
		MeshColliderType m_type;
	};
}

#endif // !MESH_COLLIDER