#ifndef RIGIDBODY
#define RIGIDBODY

#include "LibMath/Transform.h"
#include "Physic.h"

namespace Apex::Physic
{
	enum class BodyType
	{
		Static,
		Dynamic,
		Kinematic
	};

	enum class ForceType
	{
		Force,
		Impulse,
		VelocityChange,
		Acceleration
	};

	struct Force
	{
		LibMath::Vector3	m_force;
		ForceType			m_type = ForceType::Force;
	};

	class RigidBodyComponent
	{
	public:
		RigidBodyComponent(float mass, BodyType type = BodyType::Dynamic);
		RigidBodyComponent(RigidBodyComponent const& other);
		RigidBodyComponent& operator=(RigidBodyComponent const& other);

		~RigidBodyComponent() = default;

		BodyType					GetType() const { return m_bodyType; }
		float						GetMass() const { return m_mass; }
		LibMath::Transform			GetTransform() const{ return m_trans; }
		LibMath::Quaternion			GetRotation() const { return m_trans.getRotation(); }
		LibMath::Vector3			GetPosition() const { return m_trans.getPosition(); }
		LibMath::Vector3			GetScale() const { return m_trans.getScale(); }
		std::vector<Force> const&	GetForces() const { return m_pendingForces; }
		// must be called in fixed update
		// force is ignored on a body that is not Dynamic
		void					AddForce(Force newForce) { m_pendingForces.push_back(newForce); }

		void					ResetForces();
		
		void SetTransform(LibMath::Transform trans);
		void SetRotation(LibMath::Quaternion rot);
		void SetPosition(LibMath::Vector3 pos);
		void SetScale(LibMath::Vector3 scale);

		void OnTriggerEnter(RigidBodyComponent* otherActor);
		void OnTriggerExit(RigidBodyComponent* otherActor);

	private:
		LibMath::Transform	m_trans;

		BodyType			m_bodyType;
		float				m_mass;

		std::vector<Force> m_pendingForces;
	};
}




#endif // !RIGIDBODY

