#include "Lighting/PointLightComponent.h"

#include "Light.h"
#include "Object.h"

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

void PointLightComponent::FillGpuData(void* data_out)
{
    GPUPointLight* data = (GPUPointLight*)data_out;

    data->m_color = m_color;
    data->m_intensity = m_intensity;
    data->m_position = GetOwner()->GetGlobalTransform().getPosition();
    data->m_radius = m_radius;
}
