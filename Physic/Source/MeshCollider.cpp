#include "MeshCollider.h"

using namespace Apex;
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

std::vector<ExposedVar> MeshCollider::GetExposedVariables()
{
    std::vector<ExposedVar> exposedVec = Collider::GetExposedVariables();

    exposedVec.insert(exposedVec.end(),
        {
            { "Mesh", ExposedVar::Mesh, &m_mesh },
            { "Type", ExposedVar::Enum, &m_type, { "Triangle", "Convex" } }
        });

    return exposedVec;
}

void MeshCollider::Serialize(std::ostream& out) const
{
    if (m_mesh.IsValid())
        out << "        \"mesh\": \"" << m_mesh->GetPath() << "\",\n";

    out << "        \"type\": \""
        << (m_type == MeshColliderType::Convex ? "Convex" : "Triangle")
        << "\",\n";

    out << "        \"isTrigger\": "
        << (m_isTrigger ? "true" : "false") << ",\n";

    out << "        \"material\": ";
    
    if (m_material.IsValid())
    {
        out << m_material->GetId() << "\n";
    }  
    else
    {
        out << "null" << "\n";
    }
}