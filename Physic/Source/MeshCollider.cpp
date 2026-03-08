#include "MeshCollider.h"

using namespace Apex::Physic;


MeshCollider::MeshCollider(MeshCollider const& other) : Collider(other.m_material)
{
	m_mesh = other.m_mesh;
	m_type = other.m_type;
}

MeshCollider& MeshCollider::operator=(MeshCollider const& other)
{
	m_material = other.m_material;
	m_type = other.m_type;
	m_mesh = other.m_mesh;

	return *this;
}
