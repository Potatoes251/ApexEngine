#include "BoxCollider.h"

using namespace Apex;
using namespace Apex::Physic;

BoxCollider::BoxCollider(BoxCollider const& other) : Collider(other.m_material, other.m_isTrigger)
{
	m_halfExtents = other.m_halfExtents;
}

BoxCollider& BoxCollider::operator=(BoxCollider const& other)
{
	m_halfExtents = other.m_halfExtents;
	m_material = other.m_material;
	m_isTrigger = other.m_isTrigger;

	return *this;
}

std::vector<ExposedVar> BoxCollider::GetExposedVariables()
{
    std::vector<ExposedVar> exposedVec = Collider::GetExposedVariables();

    exposedVec.insert(exposedVec.end(),
        {
            { "Half extents", ExposedVar::Vector3,  &m_halfExtents}
        });

    return exposedVec;
}

void BoxCollider::DrawVariables(Apex::UserInterface::IGUI& gui)
{
    gui.Text("Half extents: (" + std::to_string(m_halfExtents[0]) + 
        ", " + std::to_string(m_halfExtents[1]) + 
        ", " + std::to_string(m_halfExtents[2]) + ")");
}

void BoxCollider::Serialize(std::ostream& out) const
{
    auto size = m_halfExtents;

    out << "      \"BoxCollider\": {\n";
    out << "        \"enabled\": " << (IsEnabled() ? "true" : "false") << ",\n";
    out << "        \"halfExtents\": ["
        << size[0] << ", "
        << size[1] << ", "
        << size[2] << "],\n";
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

    out << "      }";
}