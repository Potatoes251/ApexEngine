#include "Lighting/PointLightComponent.h"

using namespace Apex;
using namespace Apex::Lighting;

void PointLightComponent::Serialize(std::ostream& out) const
{
    out << "      \"DirectionalLightComponent\": {\n";

    out << "        \"enabled\": " << (IsEnabled() ? "true" : "false") << ",\n";
    out << "        \"color\": ["
        << m_color[0] << ", "
        << m_color[1] << ", "
        << m_color[2] << "],\n";
    out << "        \"intensity\": " << m_intensity << ",\n";
    out << "        \"radius\": " << m_radius << "\n";

    out << "      }";
}

std::vector<ExposedVar> PointLightComponent::GetExposedVariables()
{
    std::vector<ExposedVar> exposedVec = LightComponent::GetExposedVariables();

    exposedVec.push_back({ "Radius", ExposedVar::Float, &m_radius });

    return exposedVec;
}
