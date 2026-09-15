#ifndef RIGIDBODY
#define RIGIDBODY

#include "LibMath/Transform.h"
#include "Physic.h"
#include "Component.h"
#include "Object.h"

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

	class RigidBodyComponent : public Component
	{
	public:
		RigidBodyComponent(float mass, BodyType type = BodyType::Dynamic);
		RigidBodyComponent(RigidBodyComponent const& other);
		RigidBodyComponent& operator=(RigidBodyComponent const& other);

		~RigidBodyComponent() = default;

		std::unique_ptr<Apex::Component>	Clone() override;

		const char* GetTypeName() const override { return "Rigidbody"; };
		std::vector<ExposedVar> GetExposedVariables() override;

		BodyType					GetType() const { return m_bodyType; }
		float						GetMass() const { return m_mass; }
		LibMath::Transform			GetTransform() const{ return GetOwner()->GetGlobalTransform(); }
		LibMath::Quaternion			GetRotation() const { return GetOwner()->GetGlobalTransform().getRotation(); }
		LibMath::Vector3			GetPosition() const { return GetOwner()->GetGlobalTransform().getPosition(); }
		LibMath::Vector3			GetScale() const { return GetOwner()->GetGlobalTransform().getScale(); }
		std::vector<Force> const&	GetForces() const { return m_pendingForces; }
		// must be called in fixed update
		// force is ignored on a body that is not Dynamic
		void						AddForce(Force newForce) { m_pendingForces.push_back(newForce); }

		void						ResetForces();
		
		void SetTransform(LibMath::Transform trans);
		void SetRotation(LibMath::Quaternion rot);
		void SetPosition(LibMath::Vector3 pos);
		void SetScale(LibMath::Vector3 scale);

		void Serialize(std::ostream& out) const override;

		bool m_lockRotationX = false;
		bool m_lockRotationY = false;
		bool m_lockRotationZ = false;
	private:
		BodyType			m_bodyType;
		float				m_mass;

		std::vector<Force> m_pendingForces;
	};
}




#endif // !RIGIDBODY

