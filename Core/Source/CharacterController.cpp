#include "CharacterController.h"

#include "Object.h"
#include "Camera.h"
#include "Physic.h"
#include "Rigidbody.h"
#include "WaterVolume.h"
#include "CapsuleCollider.h"

#include "Application.h"

#include "LibMath/Trigonometry.h"
#include "LibMath/Arithmetic.h"
#include "LibMath/Transform.h"

#include "Log.h"

#include "InputSystem.h"

using namespace Apex::Data;
using namespace Apex::Physic;
using namespace Apex::Controller;

float CharacterController::GetHorizontalVelocity() const
{
	LibMath::Vector3 horizontalVelocity = m_velocity;
	horizontalVelocity[1] = 0.f;
	return horizontalVelocity.magnitude();
}

void CharacterController::Teleport(LibMath::Transform const& newPos)
{
	GetOwner()->SetGlobalTransform(newPos);

	m_targetPos = newPos.getPosition();
	m_targetYaw = LibMath::Degree(newPos.getEulerRotation()[1]);

	m_grounded = false;
	m_inWater = false;

	Rendering::Camera* cam = GetOwner()->GetComponent<Rendering::Camera>();
	if (cam) cam->Teleport(m_targetPos);
}

void CharacterController::Teleport(LibMath::Vector3 const& newPos)
{
	LibMath::Transform trans = GetOwner()->GetGlobalTransform();
	trans.setPosition(newPos);
	Teleport(trans);
}

void CharacterController::Move(LibMath::Vector3 const& movement)
{
	m_moveInput += movement;
}

void CharacterController::Jump()
{
	m_verticalVelocity = m_jumpForce;
}

void CharacterController::OnStart()
{
	Object* owner = GetOwner();

	if (!owner) return;

	m_body = owner->GetComponent<RigidBodyComponent>();
	m_collider = owner->GetComponent<CapsuleCollider>();
	m_targetPos = owner->GetLocalTransform().getPosition();
}

void CharacterController::OnFixedUpdate(float deltatime_s)
{
	UpdateGrounded();
	if (!m_collider) return;

	m_previousPos = m_targetPos;

	LibMath::Vector3 displacement = CalculateDisplacement(deltatime_s);

	if (displacement.magnitudeSquared() == 0.f) 
	{
		m_velocity = {};
		return;
	}

	LibMath::Vector3 remaining = displacement;
	constexpr int MAX_ITERATIONS = 3;

	for (int i = 0; i < MAX_ITERATIONS; i++)
	{
		if (remaining.magnitudeSquared() == 0.f) break;

		std::vector<HitResult> hits;
		HitResult hit;

		LibMath::Vector3 startPos = m_targetPos - LibMath::Vector3{ 0, m_collider->GetHalfHeight(), 0 };
		if (m_physic->Sweep(hits, startPos, remaining.normalized(), *m_collider, remaining.magnitude())
			&& FindClosestHit(hits, hit))
		{
			m_targetPos += remaining.normalized() * (hit.m_distance - .01f);

			// Project remaining onto the hit surface
			remaining = remaining - remaining.normalized() * hit.m_distance;
			remaining = remaining - hit.m_normal * remaining.dot(hit.m_normal);
		}
		else
		{
			m_targetPos += remaining;
			break; // No hit, consumed all movement
		}
	}

	m_velocity = (m_targetPos - m_previousPos) / deltatime_s;

	Focus(deltatime_s);

	m_moveInput = LibMath::Vector3(0.0f);
}

void CharacterController::OnUpdate(float deltatime_s)
{
	Object* owner = GetOwner();

	if (!owner) return;

	LibMath::Vector3 pos = owner->GetLocalTransform().getPosition();

	float smoothSpeed = 10.0f; // higher = faster, less lag
	float lerpFactor = 1.0f - exp(-smoothSpeed * deltatime_s);

	owner->SetLocalPosition(LibMath::Vector3::lerp(pos, m_targetPos, lerpFactor));
	owner->SetLocalRotation({ LibMath::Radian(), m_targetYaw, LibMath::Radian() });
}

void CharacterController::Serialize(std::ostream& out) const
{
	out << "        \"groundedDistance\": " << m_groundedDistance << ",\n";
	out << "        \"speed\": " << m_speed << ",\n";
	out << "        \"rotationSpeed\": " << m_rotationSpeed << ",\n";
	out << "        \"density\": " << m_density << "\n";
}

std::vector<Apex::ExposedVar> CharacterController::GetExposedVariables()
{
	return 
	{	
		{ "Distance to ground", ExposedVar::Float, &m_groundedDistance },
		{ "Speed", ExposedVar::Float, &m_speed },
		{ "Rotation Speed", ExposedVar::Float, &m_rotationSpeed },
		{ "Density", ExposedVar::Float, &m_density },
	};
}

bool CharacterController::FindClosestHit(std::vector<HitResult> const& hits, HitResult& closest_out) const
{
	bool found = false;
	for (HitResult const& hit : hits)
	{
		Object* obj = Application::Get()->GetScene()->GetObjectWithId(hit.m_objId);
		if (!obj) continue;
		Collider* collider = dynamic_cast<Collider*>(obj->GetComponent(hit.m_colliderId));
		RigidBodyComponent* body = dynamic_cast<RigidBodyComponent*>(obj->GetComponent(hit.m_bodyId));
		if (body != m_body
			&& !collider->IsTrigger()
			&& (!found || hit.m_distance < closest_out.m_distance))
		{
			closest_out = hit;
			found = true;
		}
	}

	return found;
}

void CharacterController::UpdateGrounded()
{
	if (!m_physic || !m_collider) return;

	std::vector<HitResult> hits;
	HitResult hit;
	
	LibMath::Vector3 rayStart = m_targetPos + LibMath::Vector3(0, m_collider->GetHalfHeight(), 0);

	m_grounded = m_physic->Sweep(hits, rayStart, LibMath::Vector3::down(), *m_collider, m_groundedDistance + m_collider->GetHalfHeight())
				&& FindClosestHit(hits, hit);
}

void CharacterController::Focus(float deltatime_s)
{
	LibMath::Vector3 horizontalVel = m_velocity;
	horizontalVel[1] = 0.0f;

	if (horizontalVel.magnitudeSquared() > 0.0001f)
	{
		horizontalVel = horizontalVel.normalized();

		LibMath::Radian targetYaw = LibMath::atan(horizontalVel[0], horizontalVel[2]);

		// Smooth rotation
		float rotationSpeed = 10.0f; // tweak
		LibMath::Radian delta = targetYaw - m_targetYaw;

		m_targetYaw += delta.radian() * std::min(rotationSpeed * deltatime_s, 1.0f);
	}
}

LibMath::Vector3 CharacterController::CalculateDisplacement(float deltatime_s)
{
	std::vector<WaterVolume*> waterVolumes = GetWaterVolumes();
	WaterVolume* currentVolume = waterVolumes.empty() ? nullptr : waterVolumes[0];

	m_inWater = currentVolume != nullptr;

	float effectiveGravity = GRAVITY[1];

	if (currentVolume)
	{
		// Buoyancy counteracts gravity based on density ratio
		float buoyancy = (currentVolume->GetDensity() / m_density)
			* -GRAVITY[1]                         // flip to get positive force
			* currentVolume->GetBuoyancyMult();
		effectiveGravity += buoyancy; // net vertical acceleration
		// clamp so character doesn't rocket upward infinitely if buoyancy > gravity
		effectiveGravity = std::min(effectiveGravity, 2.f);
	}

	m_verticalVelocity += effectiveGravity * deltatime_s;

	if (m_grounded && m_verticalVelocity < 0.f)
		m_verticalVelocity = 0.f;

	// Build velocity
	LibMath::Vector3 velocity = m_moveInput * m_speed;

	m_verticalVelocity += velocity[1];

	if (currentVolume)
	{
		constexpr float MAX_SWIM_VERTICAL = 5.f;
		m_verticalVelocity = LibMath::clamp(m_verticalVelocity, -MAX_SWIM_VERTICAL, MAX_SWIM_VERTICAL);
	}
	else
	{
		constexpr float MAX_FALL_SPEED = -50.f;
		m_verticalVelocity = LibMath::clamp(m_verticalVelocity, MAX_FALL_SPEED, -MAX_FALL_SPEED);
	}

	if (currentVolume)
	{
		// Add water current to horizontal movement
		velocity += currentVolume->GetFlowDir().normalized() * currentVolume->GetFlowSpeed();

		// Apply drag to all axes
		float dragFactor = std::pow(1.f - currentVolume->GetDrag(), deltatime_s);
		velocity *= dragFactor;
		m_verticalVelocity *= dragFactor;
	}

	velocity[1] = m_verticalVelocity;

	return velocity * deltatime_s;
}

std::vector<WaterVolume*> CharacterController::GetWaterVolumes() const
{
	std::vector<WaterVolume*> res;

	for (WaterVolume* vol : Application::Get()->GetScene()->GetComponents<WaterVolume>())
	{
		if (vol->IsInside(m_targetPos))
		{
			res.push_back(vol);
		}
	}

	return res;
}
