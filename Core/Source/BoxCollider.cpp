#include "BoxCollider.h"

using namespace Apex::Physic;

BoxCollider::BoxCollider(BoxCollider const& other)
{
	m_halfExtents = other.m_halfExtents;
}

BoxCollider& BoxCollider::operator=(BoxCollider const& other)
{
	m_halfExtents = other.m_halfExtents;

	return *this;
}
