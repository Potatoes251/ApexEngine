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


	class RigidBodyComponent : public Apex::Component
	{
	public:
		RigidBodyComponent(float mass, BodyType type = BodyType::Dynamic);
		RigidBodyComponent(const RigidBodyComponent&) = delete;
		RigidBodyComponent& operator=(const RigidBodyComponent&) = delete;

		~RigidBodyComponent() = default;

		BodyType			GetType() const { return m_bodyType; }
		float				GetMass() const { return m_mass; }

		LibMath::Transform&			GetTransform() { return GetOwner()->GetLocalTransform(); }
		const LibMath::Transform&	GetTransform() const{ return GetOwner()->GetLocalTransform(); }
		LibMath::Quaternion			GetRotation() const { return GetOwner()->GetLocalTransform().getRotation(); }
		LibMath::Vector3			GetPosition() const { return GetOwner()->GetLocalTransform().getPosition(); }
		LibMath::Vector3			GetScale() const { return GetOwner()->GetLocalTransform().getScale(); }
		
		
		void SetTransform(LibMath::Transform trans);
		void SetRotation(LibMath::Quaternion rot);
		void SetPosition(LibMath::Vector3 pos);
		void SetScale(LibMath::Vector3 scale);

	private:
		BodyType	m_bodyType;
		float		m_mass;
	};
}




#endif // !RIGIDBODY

