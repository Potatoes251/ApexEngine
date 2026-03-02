#ifndef COLLIDER
#define COLLIDER

namespace Apex::Physic
{
	enum class ColliderType
	{
		Box,
		Capsule,
		Mesh
	};

	class Collider
	{
	public:
		virtual ~Collider() = default;

		virtual ColliderType GetType() const = 0;
	};
}

#endif // !COLLIDER