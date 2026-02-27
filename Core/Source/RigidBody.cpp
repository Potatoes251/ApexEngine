#include "Rigidbody.h"

using namespace Apex::Physic;

RigidBodyComponent::RigidBodyComponent(float mass, BodyType type)
{
	m_mass = mass;
	m_bodyType = type;
}

RigidBodyComponent::RigidBodyComponent(RigidBodyComponent const& other)
{
	m_pos = other.m_pos;
	m_mass = other.m_mass;
	m_bodyType = other.m_bodyType;
}

RigidBodyComponent& RigidBodyComponent::operator=(RigidBodyComponent const& other)
{
	m_pos = other.m_pos;
	m_mass = other.m_mass;
	m_bodyType = other.m_bodyType;
}

void RigidBodyComponent::SetPos(LibMath::Vector3 pos)
{
	m_pos = pos;
}
