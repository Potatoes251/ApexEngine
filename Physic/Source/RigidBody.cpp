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
	m_pendingForces = other.m_pendingForces;
}

RigidBodyComponent& RigidBodyComponent::operator=(RigidBodyComponent const& other)
{
	if (this != &other)
	{
		m_mass = other.m_mass;
		m_bodyType = other.m_bodyType;
		m_pendingForces = other.m_pendingForces;
	}
	return *this;
}

std::unique_ptr<Apex::Component> RigidBodyComponent::Clone() const
{
	auto clone = std::make_unique<RigidBodyComponent>(*this);
	return clone;
}

std::vector<ExposedVar> RigidBodyComponent::GetExposedVariables()
{
	return 
	{
		{ "BodyType", ExposedVar::Enum, &m_bodyType, { "Static", "Dynamic", "Kinematic" } },
		{ "Mass", ExposedVar::Float, &m_mass }
	};
}

void RigidBodyComponent::DrawVariables(Apex::UserInterface::IGUI& gui)
{
	std::string bodyType;
	switch (m_bodyType)
	{
	case Apex::Physic::BodyType::Static:
		bodyType = "Static";
		break;
	case Apex::Physic::BodyType::Dynamic:
		bodyType = "Dynamic";
		break;
	case Apex::Physic::BodyType::Kinematic:
		bodyType = "Kinematic";
		break;
	default:
		break;
	}

	gui.Text("Body type: " + bodyType);
	gui.SliderFloat("Mass", &m_mass, -1000.f, 1000.f);
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
	LibMath::Transform& transform = GetOwner()->GetLocalTransform();
	transform.setRotation(rot);
}

void RigidBodyComponent::SetPosition(LibMath::Vector3 pos)
{
	LibMath::Transform& transform = GetOwner()->GetLocalTransform();
	transform.setPosition(pos);
}

void RigidBodyComponent::SetScale(LibMath::Vector3 scale)
{
	LibMath::Transform& transform = GetOwner()->GetLocalTransform();
	transform.setScale(scale);
}

void RigidBodyComponent::OnTriggerEnter(RigidBodyComponent* otherActor)
{
	printf("OnTriggerEnter\n");
}

void RigidBodyComponent::OnTriggerExit(RigidBodyComponent* otherActor)
{
	printf("OnTriggerExit\n");
}

void RigidBodyComponent::Serialize(std::ostream& out) const
{
	out << "      \"RigidBody\": {\n";
	out << "        \"enabled\": " << (IsEnabled() ? "true" : "false") << ",\n";
	out << "        \"type\": \"";

	switch (m_bodyType)
	{
	case BodyType::Static: out << "Static"; break;
	case BodyType::Dynamic: out << "Dynamic"; break;
	case BodyType::Kinematic: out << "Kinematic"; break;
	}

	out << "\",\n";
	out << "        \"mass\": " << m_mass << "\n";
	out << "      }";
}