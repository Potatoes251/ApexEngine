#include "CapsuleCollider.h"

using namespace Apex;
using namespace Apex::Physic;

CapsuleCollider::CapsuleCollider(CapsuleCollider const& other) : Collider(other.m_material, other.m_isTrigger)
{
	m_halfHeight = other.m_halfHeight;
	m_radius = other.m_radius;
}

CapsuleCollider& Apex::Physic::CapsuleCollider::operator=(CapsuleCollider const& other)
{
	m_halfHeight = other.m_halfHeight;
	m_radius = other.m_radius;
	m_material = other.m_material;
	m_isTrigger = other.m_isTrigger;

	return *this;
}

std::vector<ExposedVar> Apex::Physic::CapsuleCollider::GetExposedVariables()
{
	std::vector<ExposedVar> exposedVec = Collider::GetExposedVariables();

	exposedVec.insert(exposedVec.end(),
		{
			{ "Half height", ExposedVar::Float, &m_halfHeight},
			{ "Radius", ExposedVar::Float, &m_radius }
		});
	
	return exposedVec;
}
