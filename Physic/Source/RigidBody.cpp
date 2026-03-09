#include "Rigidbody.h"

using namespace Apex::Physic;

RigidBodyComponent::RigidBodyComponent(float mass, BodyType type)
{
	m_mass = mass;
	m_bodyType = type;
}

void RigidBodyComponent::ResetForces()
{
	m_pendingForces.clear();
	m_pendingForces.shrink_to_fit();
}

void RigidBodyComponent::SetTransform(LibMath::Transform trans)
{
	GetOwner()->SetLocalTransform(trans);
}

void RigidBodyComponent::SetRotation(LibMath::Quaternion rot)
{
	auto transform = GetOwner()->GetLocalTransform();
	transform.setRotation(rot);
	GetOwner()->SetLocalTransform(transform);
}

void RigidBodyComponent::SetPosition(LibMath::Vector3 pos)
{
	auto transform = GetOwner()->GetLocalTransform();
	transform.setPosition(pos);
	GetOwner()->SetLocalTransform(transform);
}

void RigidBodyComponent::SetScale(LibMath::Vector3 scale)
{
	auto transform = GetOwner()->GetLocalTransform();
	transform.setScale(scale);
	GetOwner()->SetLocalTransform(transform);
}

void RigidBodyComponent::OnTriggerEnter(RigidBodyComponent* otherActor)
{
	printf("OnTriggerEnter\n");
}

void RigidBodyComponent::OnTriggerExit(RigidBodyComponent* otherActor)
{
	printf("OnTriggerExit\n");
}
