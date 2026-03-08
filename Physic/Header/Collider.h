#ifndef COLLIDER
#define COLLIDER

#include "ResourceHandle.h"

#include "PhysicMaterial.h"

namespace Apex::Physic
{
	using PhysMatHandle = Resources::ResourceHandle<PhysicMaterial>;

	enum class ColliderType
	{
		Box,
		Capsule,
		Mesh
	};

	class Collider
	{
	public:
		Collider(PhysMatHandle material) : m_material(material) {}
		virtual ~Collider() = default;

		virtual ColliderType GetType() const = 0;

		void SetMaterial(PhysMatHandle material) { m_material = material; }
		PhysMatHandle GetMaterial() const { return m_material; }

	protected:
		PhysMatHandle m_material;
	};
}

#endif // !COLLIDER