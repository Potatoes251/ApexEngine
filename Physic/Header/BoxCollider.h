#ifndef BOX_COLLIDER
#define BOX_COLLIDER

#include "LibMath/Vector/Vector3.h"

#include "Collider.h"

namespace Apex::Physic
{
	class BoxCollider : public Collider
	{
	public:
		BoxCollider(LibMath::Vector3 const& halfExtents, bool isTrigger, PhysMatHandle material = PhysMatHandle())
			: m_halfExtents(halfExtents), Collider(material, isTrigger) {}
		BoxCollider(BoxCollider const&);
		BoxCollider& operator=(BoxCollider const&);

		~BoxCollider() = default;

		ColliderType GetType() const override { return ColliderType::Box; }

		LibMath::Vector3 GetHalfExtents() const { return m_halfExtents; }

	private:
		LibMath::Vector3 m_halfExtents;
	};
}


#endif // !BOX_COLLIDER

