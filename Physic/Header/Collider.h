#ifndef COLLIDER
#define COLLIDER

#include "ResourceHandle.h"

#include "PhysicMaterial.h"

#include "Component.h"

namespace Apex::Physic
{
	using PhysMatHandle = Resources::ResourceHandle<PhysicMaterial>;

	enum class ColliderType
	{
		Box,
		Capsule,
		Mesh
	};

	class Collider : public Apex::Component
	{
	public:
		Collider(PhysMatHandle material, bool isTrigger) : m_material(material), m_isTrigger(isTrigger) {}
		virtual ~Collider() = default;

		virtual ColliderType GetType() const = 0;

		void SetMaterial(PhysMatHandle material) { m_material = material; }
		PhysMatHandle GetMaterial() const { return m_material; }

		bool IsTrigger() const { return m_isTrigger; }

	protected:
		PhysMatHandle	m_material;
		bool			m_isTrigger;
	};
}

#endif // !COLLIDER