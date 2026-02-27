#include "Collider.h"

using namespace Apex::Physic;

Collider::Collider(Collider const& other)
{
	m_halfExtents = other.m_halfExtents;
}

Collider& Collider::operator=(Collider const& other)
{
	m_halfExtents = other.m_halfExtents;

	return *this;
}
