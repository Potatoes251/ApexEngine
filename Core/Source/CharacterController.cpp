#include "CharacterController.h"

#include "Object.h"
#include "Physic.h"
#include "Rigidbody.h"
#include "WaterVolume.h"
#include "CapsuleCollider.h"

#include "Application.h"

#include "LibMath/Trigonometry.h"
#include "LibMath/Arithmetic.h"

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

void CharacterController::Move(LibMath::Vector3 movement)
{
	m_moveInput += movement;
}

void CharacterController::Jump()
{
	if (m_grounded)
	{
		m_verticalVelocity = m_jumpForce;
	}
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

	std::vector<HitResult> hits;
	HitResult hit;

	if (m_physic->Sweep(hits, m_targetPos, displacement.normalized(), *m_collider, displacement.magnitude()) && FindClosestHit(hits, hit))
	{
		// Move only up to contact point
		LibMath::Vector3 safeMove = displacement.normalized() * hit.m_distance;
		m_targetPos += safeMove;

		// Slide along surface
		LibMath::Vector3 remaining = displacement - safeMove;
		LibMath::Vector3 slide = remaining - hit.m_normal * remaining.dot(hit.m_normal);

		m_targetPos += slide;
	}
	else
	{
		m_targetPos += displacement;
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
	out << "      \"CharacterController\": {\n";
	out << "        \"enabled\": " << (IsEnabled() ? "true" : "false") << ",\n";
	out << "        \"groundedDistance\": " << m_groundedDistance << ",\n";
	out << "        \"speed\": " << m_speed << ",\n";
	out << "        \"rotationSpeed\": " << m_rotationSpeed << ",\n";
	out << "        \"density\": " << m_density << "\n";

	out << "      }";
}

std::vector<Apex::ExposedVar> CharacterController::GetExposedVariables()
{
	return { { "Distance to ground", ExposedVar::Float, &m_groundedDistance },
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
		if (hit.m_body != m_body 
			&& !hit.m_collider->IsTrigger()
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

	m_grounded = m_physic->Raycast(hits, rayStart, LibMath::Vector3::down(), m_groundedDistance + m_collider->GetHalfHeight())
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
		// Assumes the character has a m_density member (e.g. 900 for a human)
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

std::vector<WaterVolume*> CharacterController::GetWaterVolumes()
{
	std::vector<WaterVolume*> res;

	for (WaterVolume* vol : Application::GetCurrentScene()->GetComponents<WaterVolume>())
	{
		if (vol->IsInside(m_targetPos))
		{
			res.push_back(vol);
		}
	}

	return res;
}
