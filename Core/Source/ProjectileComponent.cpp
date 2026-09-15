#include "ProjectileComponent.h"

#include "Object.h"
#include "Physic.h"
#include "Log.h"

using namespace Apex::Gameplay;

Apex::Gameplay::ProjectileComponent::ProjectileComponent(LibMath::Vector3 const& direction, float speed, float lifeSpan) :
	m_direction(direction.normalized()), m_speed(speed), m_lifeSpan(lifeSpan) {}

Apex::Gameplay::ProjectileComponent::ProjectileComponent(ProjectileComponent const& other) : 
	m_direction(other.m_direction), m_speed(other.m_speed), m_lifeSpan(other.m_lifeSpan), m_age(other.m_age) {}

void Apex::Gameplay::ProjectileComponent::Setup(LibMath::Vector3 const& direction, float speed, float lifeSpan)
{
    m_direction = direction.normalized();
    m_speed = speed;
    m_lifeSpan = lifeSpan;
}

void ProjectileComponent::OnFixedUpdate(float fixedDeltaTime_s)
{
    m_age += fixedDeltaTime_s;
    if (m_age >= m_lifeSpan)
    {
        GetOwner()->Destroy();
        return;
    }

    LibMath::Vector3 displacement = m_direction * m_speed * fixedDeltaTime_s;
    LibMath::Vector3 newPosition = GetOwner()->GetLocalTransform().getPosition() + displacement;
    GetOwner()->SetLocalPosition(newPosition);
}

void Apex::Gameplay::ProjectileComponent::OnTriggerEnter(Physic::Collider* other)
{
	GetOwner()->Destroy();
}

void ProjectileComponent::Serialize(std::ostream& out) const 
{
	out << "        \"direction\": [" << m_direction[0] << ", " << m_direction[1] << ", " << m_direction[2] << "],\n";
	out << "        \"speed\": " << m_speed << ",\n";
	out << "        \"lifeSpan\": " << m_lifeSpan << "\n";
}
std::vector<Apex::ExposedVar> ProjectileComponent::GetExposedVariables() 
{ 
    return 
    {
        { "Direction", ExposedVar::Vector3,& m_direction },
	    { "Speed", ExposedVar::Float, &m_speed },
        { "Life Span", ExposedVar::Float, &m_lifeSpan }
    }; 
}