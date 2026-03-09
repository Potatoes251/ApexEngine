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
		virtual ~Collider() = default;

		virtual ColliderType GetType() const = 0;
	};
}

#endif // !COLLIDER