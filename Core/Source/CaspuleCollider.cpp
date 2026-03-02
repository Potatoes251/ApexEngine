#include "CaspuleCollider.h"

using namespace Apex::Physic;

CapsuleCollider::CapsuleCollider(CapsuleCollider const& other)
{
	m_halfHeight = other.m_halfHeight;
	m_radius = other.m_radius;
}

CapsuleCollider& Apex::Physic::CapsuleCollider::operator=(CapsuleCollider const& other)
{
	m_halfHeight = other.m_halfHeight;
	m_radius = other.m_radius;

	return *this;
}
