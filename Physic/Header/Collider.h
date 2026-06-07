#ifndef COLLIDER
#define COLLIDER

#include "Component.h"

namespace Apex::Physic
{
	enum class ColliderType
	{
		Box,
		Capsule,
		Mesh
	};

	class Collider : public Apex::Component
	{
	public:
		Collider(bool isTrigger) : m_isTrigger(isTrigger) {}
		virtual ~Collider() = default;

		virtual ColliderType GetType() const = 0;
		virtual std::vector<ExposedVar> GetExposedVariables() override;
		virtual void Serialize(std::ostream& out) const override;

		bool IsTrigger() const { return m_isTrigger; }

		size_t GetMatId() const { return m_id; }

		float	GetStaticFriction() const { return m_staticFriction; }
		void	SetStaticFriction(float staticFriction) { m_staticFriction = staticFriction; }

		float	GetDynamicFriction() const { return m_dynamicFriction; }
		void	SetDynamicFriction(float dynamicFriction) { m_dynamicFriction = dynamicFriction; }

		float	GetBounciness() const { return m_bounciness; }
		void	SetBounciness(float bounciness) { m_bounciness = bounciness; }

	protected:
		float m_staticFriction = 0.5f;
		float m_dynamicFriction = 0.5f;
		float m_bounciness = 0.f;

		// id of physics material
		size_t m_id = SIZE_MAX;

		bool m_isTrigger;
	};
}

#endif // !COLLIDER