#include "BoxCollider.h"

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