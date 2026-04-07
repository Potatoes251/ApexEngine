#include "CharacterController.h"

#include "Object.h"
#include "Rigidbody.h"
#include "Physic.h"
#include "CapsuleCollider.h"

using namespace Apex::Controller;
using namespace Apex::Physic;

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
}

void CharacterController::OnFixedUpdate(float deltatime_s)
{
	UpdateGrounded();
	Object* owner = GetOwner();

	if (!owner || !m_collider) return;

	LibMath::Vector3 pos = owner->GetLocalTransform().getPosition();

	m_verticalVelocity += GRAVITY[1] * deltatime_s;
	if (m_grounded && m_verticalVelocity < 0.f)
		m_verticalVelocity = 0.f;

	LibMath::Vector3 velocity = m_moveInput * m_speed;
	velocity[1] = m_verticalVelocity;
	LibMath::Vector3 displacement = velocity * deltatime_s;

	if (displacement.magnitudeSquared() == 0.f) return;

	std::vector<HitResult> hits;
	HitResult hit;

	if (m_physic->Sweep(hits, pos, displacement.normalized(), *m_collider, m_speed) && FindClosestHit(hits, hit))
	{
		// Move only up to contact point
		LibMath::Vector3 safeMove = displacement.normalized() * hit.m_distance;
		pos += safeMove;

		// Slide along surface
		LibMath::Vector3 remaining = displacement - safeMove;
		LibMath::Vector3 slide = remaining - hit.m_normal * remaining.dot(hit.m_normal);

		pos += slide;
	}
	else
	{
		pos += displacement;
	}

	owner->SetLocalPosition(pos);
	m_moveInput = LibMath::Vector3(0.0f);
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
	return { { "distance to ground", ExposedVar::Float, &m_groundedDistance },
			{ "speed", ExposedVar::Float, &m_speed } };
}

bool CharacterController::FindClosestHit(std::vector<HitResult> const& hits, HitResult& closest_out) const
{
	bool found = false;
	for (HitResult const& hit : hits)
	{
		if (hit.m_body != m_body && (!found || hit.m_distance < closest_out.m_distance))
		{
			closest_out = hit;
			found = true;
		}
	}

	return found;
}

void CharacterController::UpdateGrounded()
{
	Object* owner = GetOwner();

	if (!m_physic || !m_collider || !owner) return;

	LibMath::Vector3 pos = owner->GetGlobalTransform().getPosition();

	std::vector<HitResult> hits;
	HitResult hit;

	m_grounded = m_physic->Raycast(hits, pos, LibMath::Vector3::down(), m_groundedDistance + m_collider->GetHalfHeight()) 
				&& FindClosestHit(hits, hit);
}
