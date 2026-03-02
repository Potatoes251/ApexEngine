#include "MeshCollider.h"

using namespace Apex::Physic;


MeshCollider::MeshCollider(MeshCollider const& other)
{
	m_mesh = other.m_mesh;
}

MeshCollider& MeshCollider::operator=(MeshCollider const& other)
{
	m_mesh = other.m_mesh;

	return *this;
}
