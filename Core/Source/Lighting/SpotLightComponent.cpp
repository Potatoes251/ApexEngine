#include "Lighting/SpotLightComponent.h"

#include "GpuLightStructs.h"
#include "Object.h"

using namespace Apex;
using namespace Apex::Lighting;

SpotLightComponent::SpotLightComponent(LibMath::Vector3 color, float intensity, float innerCutoff, float outerCutoff)
    : LightComponent(color, intensity) 
{
    m_innerCutoff = innerCutoff;
    m_outerCutoff = outerCutoff;
}

SpotLightComponent::SpotLightComponent(SpotLightComponent const& other)
{
    m_color = other.m_color;
    m_intensity = other.m_intensity;
    m_innerCutoff = other.m_innerCutoff;
    m_outerCutoff = other.m_outerCutoff;
}

SpotLightComponent& SpotLightComponent::operator=(SpotLightComponent const& other)
{
    m_color = other.m_color;
    m_intensity = other.m_intensity;
    m_innerCutoff = other.m_innerCutoff;
    m_outerCutoff = other.m_outerCutoff;

    return *this;
}

void SpotLightComponent::Serialize(std::ostream& out) const
{
    out << "      \"SpotLight\": {\n";

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

    exposedVec.push_back({ "Inner angle", ExposedVar::Float, &m_innerCutoff });
    exposedVec.push_back({ "Outer angle", ExposedVar::Float, &m_outerCutoff });

    return exposedVec;
}

void SpotLightComponent::FillGpuData(void* data_out)
{
    GPUSpotLight* data = (GPUSpotLight*)data_out;

    data->m_color = { m_color, 0 };
    data->m_direction = { GetOwner()->GetGlobalTransform().getForward(), 0 };
    data->m_position = { GetOwner()->GetGlobalTransform().getPosition(), 0 };
    data->m_param = { m_intensity, m_innerCutoff, m_outerCutoff,0 };
}
