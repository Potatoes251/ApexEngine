#include "Lighting/DirectionalLightComponent.h"

using namespace Apex;
using namespace Apex::Lighting;

void DirectionalLightComponent::Serialize(std::ostream& out) const
{
    out << "      \"DirectionalLightComponent\": {\n";

    out << "        \"enabled\": " << (IsEnabled() ? "true" : "false") << ",\n";
    out << "        \"color\": ["
        << m_color[0] << ", "
        << m_color[1] << ", "
        << m_color[2] << "],\n";
    out << "        \"intensity\": " << m_intensity << "\n";

    out << "      }";
}

std::vector<ExposedVar> DirectionalLightComponent::GetExposedVariables()
{
    return LightComponent::GetExposedVariables();
}
