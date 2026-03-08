#include "CapsuleCollider.h"

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
