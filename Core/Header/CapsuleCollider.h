#ifndef CAPSULE_COLLIDER
#define CAPSULE_COLLIDER

#include "LibMath/Vector/Vector3.h"

#include "Collider.h"

namespace Apex::Physic
{
	class CapsuleCollider : public Collider
	{
	public:
		CapsuleCollider(float halfHeight, float radius) : m_halfHeight(halfHeight), m_radius(radius) {}
		CapsuleCollider(CapsuleCollider const& other);
		CapsuleCollider& operator=(CapsuleCollider const& other);

		ColliderType GetType() const override { return ColliderType::Capsule; }

		float GetHalfHeight() const { return m_halfHeight; }
		float GetRadius() const { return m_radius; }

	private:
		float m_halfHeight;
		float m_radius;

	};
}


#endif // !CAPSULE_COLLIDER