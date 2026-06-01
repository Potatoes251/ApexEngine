#include "Rigidbody.h"

using namespace Apex;
using namespace Apex::Physic;

RigidBodyComponent::RigidBodyComponent(float mass, BodyType type)
{
	m_mass = mass;
	m_bodyType = type;
}

RigidBodyComponent::RigidBodyComponent(RigidBodyComponent const& other)
{
	m_mass = other.m_mass;
	m_bodyType = other.m_bodyType;
	m_lockRotationX = other.m_lockRotationX;
	m_lockRotationY = other.m_lockRotationY;
	m_lockRotationZ = other.m_lockRotationZ;
}

RigidBodyComponent& RigidBodyComponent::operator=(RigidBodyComponent const& other)
{
	m_mass = other.m_mass;
	m_bodyType = other.m_bodyType;
	m_lockRotationX = other.m_lockRotationX;
	m_lockRotationY = other.m_lockRotationY;
	m_lockRotationZ = other.m_lockRotationZ;
	return *this;
}

std::unique_ptr<Apex::Component> RigidBodyComponent::Clone()
{
	auto clone = std::make_unique<RigidBodyComponent>(*this);
	return clone;
}

std::vector<ExposedVar> RigidBodyComponent::GetExposedVariables()
{
	return 
	{
		{ "BodyType", ExposedVar::Enum, &m_bodyType, { "Static", "Dynamic", "Kinematic" } },
		{ "Mass", ExposedVar::Float, &m_mass },
		{ "Lock Rotation X", ExposedVar::Bool, &m_lockRotationX },
		{ "Lock Rotation Y", ExposedVar::Bool, &m_lockRotationY },
		{ "Lock Rotation Z", ExposedVar::Bool, &m_lockRotationZ }
	};
}

void RigidBodyComponent::ResetForces()
{
	m_pendingForces.clear();
}

void RigidBodyComponent::SetTransform(LibMath::Transform trans)
{
	GetOwner()->SetLocalTransform(trans);
}

void RigidBodyComponent::SetRotation(LibMath::Quaternion rot)
{
	LibMath::Transform transform = GetOwner()->GetLocalTransform();
	transform.setRotation(rot);
	GetOwner()->SetLocalTransform(transform);
}

void RigidBodyComponent::SetPosition(LibMath::Vector3 pos)
{
	LibMath::Transform transform = GetOwner()->GetLocalTransform();
	transform.setPosition(pos);
	GetOwner()->SetLocalTransform(transform);
}

void RigidBodyComponent::SetScale(LibMath::Vector3 scale)
{
	LibMath::Transform transform = GetOwner()->GetLocalTransform();
	transform.setScale(scale);
	GetOwner()->SetLocalTransform(transform);
}

void RigidBodyComponent::Serialize(std::ostream& out) const
{
	out << "        \"type\": \"";

	switch (m_bodyType)
	{
	case BodyType::Static: out << "Static"; break;
	case BodyType::Dynamic: out << "Dynamic"; break;
	case BodyType::Kinematic: out << "Kinematic"; break;
	}

	out << "\",\n";
	out << "        \"mass\": " << m_mass << "\n";
}