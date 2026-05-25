#include "BoxCollider.h"

using namespace Apex;
using namespace Apex::Physic;

std::vector<ExposedVar> BoxCollider::GetExposedVariables()
{
    std::vector<ExposedVar> exposedVec = Collider::GetExposedVariables();

    exposedVec.insert(exposedVec.end(),
        {
            { "Half extents", ExposedVar::Vector3,  &m_halfExtents}
        });

    return exposedVec;
}

void BoxCollider::Serialize(std::ostream& out) const
{
    Collider::Serialize(out);
    out << "        \"halfExtents\": " << m_halfExtents << "\n";
}