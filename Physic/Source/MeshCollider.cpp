#include "MeshCollider.h"

using namespace Apex::Physic;


MeshCollider::MeshCollider(MeshCollider const& other) : Collider(other.m_material, other.m_isTrigger)
{
	m_mesh = other.m_mesh;
	m_type = other.m_type;
}

MeshCollider& MeshCollider::operator=(MeshCollider const& other)
{
	m_type = other.m_type;
	m_mesh = other.m_mesh;
	m_material = other.m_material;
	m_isTrigger = other.m_isTrigger;

	return *this;
}
