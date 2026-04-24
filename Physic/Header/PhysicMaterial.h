#ifndef PHYSIC_MATERIAL
#define PHYSIC_MATERIAL

#include "IResource.h"

class PhysicMaterial : public IResource
{
public:
	PhysicMaterial() = default;
	PhysicMaterial(float staticFriction, float dynamicFriction, float bounciness);
	PhysicMaterial(PhysicMaterial const& other);
	PhysicMaterial& operator=(PhysicMaterial const& other);
	
	void	SetName(std::string const& name) { m_path = name; }

	size_t	GetId() const { return m_id; }
	void	SetId(size_t newId) { m_id = newId; }
	
	float	GetStaticFriction() const { return m_staticFriction; }
	float&	MutStaticFriction() { return m_staticFriction; }
	void	SetStaticFriction(float staticFriction) { m_staticFriction = staticFriction; }

	float	GetDynamicFriction() const { return m_dynamicFriction; }
	float&	MutDynamicFriction() { return m_dynamicFriction; }
	void	SetDynamicFriction(float dynamicFriction) { m_dynamicFriction = dynamicFriction; }
	
	float	GetBounciness() const { return m_bounciness; }
	float&	MutBounciness() { return m_bounciness; }
	void	SetBounciness(float bounciness) { m_bounciness = bounciness; }

private:
	// how hard to start moving
	float m_staticFriction = 0.5f;
	// how it slow down the object while moving
	float m_dynamicFriction = 0.5f;
	// how much it bounces back
	float m_bounciness = 0.f;

	// id of physics material
	size_t m_id = SIZE_MAX;
};

#endif // !PHYSIC_MATERIAL
