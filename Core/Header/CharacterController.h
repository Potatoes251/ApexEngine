#ifndef CHARACTER_CONTROLLER
#define CHARACTER_CONTROLLER

#include "Component.h"

#include "LibMath/Vector/Vector3.h"
#include "Physic.h"

namespace Apex::Physic 
{ 
	class RigidBodyComponent; 
	class CapsuleCollider;
	class WaterVolume;
}

namespace Apex::Controller
{
	class CharacterController : public Component
	{
	public:
		CharacterController() = default;
		CharacterController(CharacterController const& other) = default;
		CharacterController& operator=(CharacterController const& other) = default;
		~CharacterController() = default;

		void SetPhysic(Physic::PhysicSystem* physic) { m_physic = physic; }
		void SetSpeed(float speed) { m_speed = speed; }
		void SetRotationSpeed(float speed) { m_rotationSpeed = speed; }
		void SetGroundedDistance(float groundedDistance) { m_groundedDistance = groundedDistance; }
		void SetDensity(float density) { m_density = density; }

		float GetHorizontalVelocity() const;
		bool IsGrounded() const { return m_grounded; }
		bool IsInWater() const { return m_inWater; }

		void Move(LibMath::Vector3 movement);
		void Jump();

		void OnStart() override;
		void OnFixedUpdate(float deltatime_s) override;
		void OnUpdate(float deltatime_s) override;

		void Serialize(std::ostream& out) const override;
		
		std::unique_ptr<Component> Clone() override { return std::make_unique<CharacterController>(*this); }
		
		const char* GetTypeName() const override { return "CharacterController"; }
		std::vector<ExposedVar> GetExposedVariables() override;

	private:
		bool FindClosestHit(std::vector<Physic::HitResult> const& hits, Physic::HitResult& closest_out) const;

		void UpdateGrounded();

		void Focus(float deltatime_s);
		LibMath::Vector3 CalculateDisplacement(float deltatime_s);

		std::vector<Physic::WaterVolume*> GetWaterVolumes();

		Physic::PhysicSystem*		m_physic = nullptr;
		Physic::RigidBodyComponent* m_body = nullptr;
		Physic::CapsuleCollider*	m_collider = nullptr;
		LibMath::Vector3			m_moveInput;
		LibMath::Vector3			m_targetPos;
		LibMath::Vector3			m_velocity;
		LibMath::Vector3			m_previousPos;
		float						m_verticalVelocity = 0.f;

		LibMath::Radian m_targetYaw;

		float m_jumpForce = 5.f;
		float m_speed = 5.f;
		float m_rotationSpeed = 5.f;
		// distance to be considered grounded
		float m_groundedDistance = 0.5f;

		float m_density = 980.f;

		bool m_grounded = false;
		bool m_inWater = false;
	};
}


#endif // !CHARACTER_CONTROLLER
