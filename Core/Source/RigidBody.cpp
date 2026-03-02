#include "Rigidbody.h"

using namespace Apex::Physic;

RigidBodyComponent::RigidBodyComponent(float mass, BodyType type)
{
	m_mass = mass;
	m_bodyType = type;
}

RigidBodyComponent::RigidBodyComponent(RigidBodyComponent const& other)
{
	m_trans = other.m_trans;
	m_mass = other.m_mass;
	m_bodyType = other.m_bodyType;
}

RigidBodyComponent& RigidBodyComponent::operator=(RigidBodyComponent const& other)
{
	m_trans = other.m_trans;
	m_mass = other.m_mass;
	m_bodyType = other.m_bodyType;

	return *this;
}

void RigidBodyComponent::SetTransform(LibMath::Transform trans)
{
	m_trans = trans;
}

void RigidBodyComponent::SetRotation(LibMath::Quaternion rot)
{
	m_trans.setRotation(rot);
}

void RigidBodyComponent::SetPosition(LibMath::Vector3 pos)
{
	m_trans.setPosition(pos);
}

void RigidBodyComponent::SetScale(LibMath::Vector3 scale)
{
	m_trans.setScale(scale);
}
