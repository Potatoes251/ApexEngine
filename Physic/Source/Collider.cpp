#include "Collider.h"

using namespace Apex;
using namespace Apex::Physic;

std::vector<ExposedVar> Collider::GetExposedVariables()
{
    return
    {
        { "Static Friction", ExposedVar::Float, &m_staticFriction  },
        { "Dynamic Friction", ExposedVar::Float, &m_dynamicFriction },
        { "Bounciness", ExposedVar::Float, &m_bounciness },
        { "Is trigger", ExposedVar::Bool, &m_isTrigger}
    };
}

void Collider::Serialize(std::ostream& out) const
{
    out << "        \"isTrigger\": " << (m_isTrigger ? "true" : "false") << ",\n";
    out << "        \"staticFriction\": " << m_staticFriction << ",\n";
    out << "        \"dynamicFriction\": " << m_dynamicFriction << ",\n";
    out << "        \"bounciness\": " << m_bounciness << ",\n";
}
