#include "ProjectileComponent.h"
#include "Object.h"
#include "Physic.h"

using namespace Apex::Gameplay;

void ProjectileComponent::Setup(Physic::PhysicSystem* physic, LibMath::Vector3 const& direction, float speed, float lifeSpan)
{
    m_physic = physic;
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
    float distance = displacement.magnitude();

    if (distance > 0.0001f && m_physic)
    {
        Physic::HitResult hit;
        // Sweeping via Raycast to check for solid obstacles
        if (m_physic->Raycast(hit, GetOwner()->GetLocalTransform().getPosition(), m_direction, distance))
        {
            // Trigger damage context or bounce here if necessary
            GetOwner()->Destroy();
            return;
        }
    }

    // Move Forward
    GetOwner()->SetLocalPosition(GetOwner()->GetLocalTransform().getPosition() + displacement);
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