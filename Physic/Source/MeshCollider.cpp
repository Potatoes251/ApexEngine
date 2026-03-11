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

void MeshCollider::Serialize(std::ostream& out) const
{
    out << "      \"MeshCollider\": {\n";

    out << "        \"enabled\": " << (IsEnabled() ? "true" : "false") << ",\n";

    if (m_mesh.IsValid())
        out << "        \"mesh\": \"" << m_mesh->GetPath() << "\",\n";

    out << "        \"type\": \""
        << (m_type == MeshColliderType::Convex ? "Convex" : "Triangle")
        << "\",\n";

    out << "        \"isTrigger\": "
        << (m_isTrigger ? "true" : "false") << ",\n";

    out << "        \"material\": " << (m_material.IsValid() ? "\"" + m_material->GetPath() + "\"" : "null") << "\n";

    out << "      }";
}