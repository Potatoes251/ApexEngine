#include "CharacterController.h"

#include "Object.h"
#include "Physic.h"
#include "Rigidbody.h"
#include "CapsuleCollider.h"

using namespace Apex::Data;
using namespace Apex::Physic;
using namespace Apex::Controller;

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

	m_verticalVelocity += GRAVITY[1] * deltatime_s;
	if (m_grounded && m_verticalVelocity < 0.f)
		m_verticalVelocity = 0.f;

	LibMath::Vector3 velocity = m_moveInput * m_speed;
	velocity[1] = m_verticalVelocity;
	LibMath::Vector3 displacement = velocity * deltatime_s;

	if (displacement.magnitudeSquared() == 0.f) return;

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
}

void CharacterController::Serialize(std::ostream& out) const
{
	out << "      \"CharacterController\": {\n";
	out << "        \"enabled\": " << (IsEnabled() ? "true" : "false") << ",\n";
	out << "        \"groundedDistance\": " << m_groundedDistance << ",\n";
	out << "        \"speed\": " << m_speed << "\n";

	out << "      }";
}

std::vector<Apex::ExposedVar> CharacterController::GetExposedVariables()
{
	return { { "Distance to ground", ExposedVar::Float, &m_groundedDistance },
			{ "Speed", ExposedVar::Float, &m_speed } };
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
