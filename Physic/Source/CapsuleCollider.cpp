#include "CapsuleCollider.h"

using namespace Apex;
using namespace Apex::Physic;

void CapsuleCollider::Serialize(std::ostream& out) const
{
	Collider::Serialize(out);
    out << "        \"halfHeight\": " << m_halfHeight<< ",\n";
    out << "        \"radius\": " << m_radius<< "\n";
}

std::vector<ExposedVar> Apex::Physic::CapsuleCollider::GetExposedVariables()
{
	std::vector<ExposedVar> exposedVec = Collider::GetExposedVariables();

	exposedVec.insert(exposedVec.end(),
		{
			{ "Half height", ExposedVar::Float, &m_halfHeight},
			{ "Radius", ExposedVar::Float, &m_radius }
		});
	
	return exposedVec;
}
