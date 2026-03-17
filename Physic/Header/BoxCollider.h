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

		std::unique_ptr<Component> Clone() const override { return std::make_unique<BoxCollider>(*this); }

		ColliderType GetType() const override { return ColliderType::Box; }
		const char* GetTypeName() const override { return "BoxCollider"; };

		LibMath::Vector3 GetHalfExtents() const { return m_halfExtents; }

		void Serialize(std::ostream& out) const override;

	private:
		LibMath::Vector3 m_halfExtents;
	};
}


#endif // !BOX_COLLIDER

