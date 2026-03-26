#include "Lighting/SpotLightComponent.h"

using namespace Apex;
using namespace Apex::Lighting;

void SpotLightComponent::Serialize(std::ostream& out) const
{
    out << "      \"DirectionalLightComponent\": {\n";

    out << "        \"enabled\": " << (IsEnabled() ? "true" : "false") << ",\n";
    out << "        \"color\": ["
        << m_color[0] << ", "
        << m_color[1] << ", "
        << m_color[2] << "],\n";
    out << "        \"intensity\": " << m_intensity << ",\n";
    out << "        \"innerCutOff\": " << m_innerCutOff << ",\n";
    out << "        \"outerCutOff\": " << m_outerCutOff << "\n";

    out << "      }";
}

std::vector<ExposedVar> SpotLightComponent::GetExposedVariables()
{
    std::vector<ExposedVar> exposedVec = LightComponent::GetExposedVariables();

    exposedVec.push_back({ "Inner angle", ExposedVar::Vector3, &m_innerCutOff });
    exposedVec.push_back({ "Outer angle", ExposedVar::Vector3, &m_outerCutOff });

    return exposedVec;
}
