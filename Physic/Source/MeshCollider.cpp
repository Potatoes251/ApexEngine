#include "MeshCollider.h"

using namespace Apex;
using namespace Apex::Physic;

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
    Collider::Serialize(out);

    if (m_mesh.IsValid())
        out << "        \"mesh\": \"" << m_mesh->GetPath() << "\",\n";

    out << "        \"type\": \""
        << (m_type == MeshColliderType::Convex ? "Convex" : "Triangle")
        << "\"\n";
}