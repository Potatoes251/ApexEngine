#include "Lighting/SpotLightComponent.h"

#include "Light.h"
#include "Object.h"

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
    out << "        \"innerCutOff\": " << m_innerCutoff << ",\n";
    out << "        \"outerCutOff\": " << m_outerCutoff << "\n";

    out << "      }";
}

std::vector<ExposedVar> SpotLightComponent::GetExposedVariables()
{
    std::vector<ExposedVar> exposedVec = LightComponent::GetExposedVariables();

    exposedVec.push_back({ "Inner angle", ExposedVar::Vector3, &m_innerCutoff });
    exposedVec.push_back({ "Outer angle", ExposedVar::Vector3, &m_outerCutoff });

    return exposedVec;
}

void SpotLightComponent::FillGpuData(void* data_out)
{
    GPUSpotLight* data = (GPUSpotLight*)data_out;

    data->m_color = m_color;
    data->m_intensity = m_intensity;
    data->m_direction = GetOwner()->GetGlobalTransform().getForward();
    data->m_position = GetOwner()->GetGlobalTransform().getPosition();
    data->m_innerCutoff = m_innerCutoff;
    data->m_outerCutoff = m_outerCutoff;
}
