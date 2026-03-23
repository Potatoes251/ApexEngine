#include "Collider.h"

using namespace Apex;
using namespace Apex::Physic;

std::vector<ExposedVar> Collider::GetExposedVariables()
{
    return
    {
        { "Material", ExposedVar::PhysMaterial, &m_material },
        { "Is trigger", ExposedVar::Bool, &m_isTrigger}
    };
}
