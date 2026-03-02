#ifndef RIGIDBODY
#define RIGIDBODY

#include "LibMath/Vector/Vector3.h"
#include "Physic.h"

namespace Apex::Physic
{
	enum class BodyType
	{
		STATIC,
		DYNAMIC,
		KINEMATIC
	};


	class RigidBodyComponent
	{
	public:
		RigidBodyComponent(float mass, BodyType type = BodyType::DYNAMIC);
		RigidBodyComponent(RigidBodyComponent const& other);
		RigidBodyComponent& operator=(RigidBodyComponent const& other);

		~RigidBodyComponent() = default;

		BodyType			GetType() const { return m_bodyType; }
		float				GetMass() const { return m_mass; }
		LibMath::Vector3	GetPosition() const { return m_pos; }
		
		
		void SetPosition(LibMath::Vector3 pos);

	private:
		LibMath::Vector3 m_pos;

		BodyType	m_bodyType;
		float		m_mass;
	};
}




#endif // !RIGIDBODY

