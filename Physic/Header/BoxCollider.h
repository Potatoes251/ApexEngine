#ifndef BOX_COLLIDER
#define BOX_COLLIDER

#include "LibMath/Vector/Vector3.h"

#include "Collider.h"

namespace Apex::Physic
{
	class BoxCollider : public Collider
	{
	public:
		BoxCollider(LibMath::Vector3 const& halfExtents, bool isTrigger)
			: m_halfExtents(halfExtents), Collider(isTrigger) {}
		BoxCollider(BoxCollider const&) = default;
		BoxCollider& operator=(BoxCollider const&) = default;

		~BoxCollider() = default;

		std::unique_ptr<Component> Clone() override { return std::make_unique<BoxCollider>(*this); }

		ColliderType GetType() const override { return ColliderType::Box; }
		const char* GetTypeName() const override { return "BoxCollider"; };
		std::vector<ExposedVar> GetExposedVariables() override;

		LibMath::Vector3 GetHalfExtents() const { return m_halfExtents; }

		void Serialize(std::ostream& out) const override;

	private:
		LibMath::Vector3 m_halfExtents;
	};
}


#endif // !BOX_COLLIDER

