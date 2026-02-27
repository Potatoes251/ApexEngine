#ifndef COLLIDER
#define COLLIDER

#include "LibMath/Vector/Vector3.h"

namespace Apex::Physic
{
	class Collider
	{
	public:
		Collider(LibMath::Vector3 const& halfExtents) : m_halfExtents(halfExtents) {}
		Collider(Collider const&);
		Collider& operator=(Collider const&);

		~Collider() = default;

		LibMath::Vector3 GetHalfExtents() const { return m_halfExtents; }

	private:
		LibMath::Vector3 m_halfExtents;
	};
}


#endif // !COLLIDER

