#include "PhysicMaterial.h"

PhysicMaterial::PhysicMaterial(float staticFriction, float dynamicFriction, float bounciness)
{
	m_staticFriction = staticFriction;
	m_dynamicFriction = dynamicFriction;
	m_bounciness = bounciness;
}

PhysicMaterial::PhysicMaterial(PhysicMaterial const& other)
{
	m_staticFriction = other.m_staticFriction;
	m_dynamicFriction = other.m_dynamicFriction;
	m_bounciness = other.m_bounciness;
	m_id = other.m_id;
}

PhysicMaterial& PhysicMaterial::operator=(PhysicMaterial const& other)
{
	m_staticFriction = other.m_staticFriction;
	m_dynamicFriction = other.m_dynamicFriction;
	m_bounciness = other.m_bounciness;
	m_id = other.m_id;
	return *this;
}
