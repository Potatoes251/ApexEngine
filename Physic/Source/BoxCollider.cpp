#include "BoxCollider.h"

using namespace Apex::Physic;

BoxCollider::BoxCollider(BoxCollider const& other) : Collider(other.m_material)
{
	m_halfExtents = other.m_halfExtents;
}

BoxCollider& BoxCollider::operator=(BoxCollider const& other)
{
	m_halfExtents = other.m_halfExtents;
	m_material = other.m_material;

	return *this;
}
