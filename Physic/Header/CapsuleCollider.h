#ifndef CAPSULE_COLLIDER
#define CAPSULE_COLLIDER

#include "LibMath/Vector/Vector3.h"

#include "Collider.h"

namespace Apex::Physic
{
	class CapsuleCollider : public Collider
	{
	public:
		CapsuleCollider(float halfHeight, float radius, bool isTrigger, PhysMatHandle material = PhysMatHandle())
			: m_halfHeight(halfHeight), m_radius(radius), Collider(material, isTrigger) {}
		CapsuleCollider(CapsuleCollider const& other);
		CapsuleCollider& operator=(CapsuleCollider const& other);

		void Serialize(std::ostream& out) const override;

		std::unique_ptr<Component> Clone() const override { return std::make_unique<CapsuleCollider>(*this); }

		const char* GetTypeName() const override { return "CapsuleCollider"; };
		ColliderType GetType() const override { return ColliderType::Capsule; }
		std::vector<ExposedVar> GetExposedVariables() override;

		void DrawVariables(Apex::UserInterface::IGUI& gui) {}

		float GetHalfHeight() const { return m_halfHeight; }
		float GetRadius() const { return m_radius; }

	private:
		float m_halfHeight;
		float m_radius;
	};
}


#endif // !CAPSULE_COLLIDER