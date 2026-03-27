#include "Lighting/PointLightComponent.h"

#include "Light.h"
#include "Object.h"

using namespace Apex;
using namespace Apex::Lighting;

PointLightComponent::PointLightComponent(LibMath::Vector3 color, float intensity, float radius)
    : LightComponent(color, intensity) 
{
    m_radius = radius;
}

PointLightComponent::PointLightComponent(PointLightComponent const& other)
{
    m_color = other.m_color;
    m_intensity = other.m_intensity;
    m_radius = other.m_radius;
}

PointLightComponent& PointLightComponent::operator=(PointLightComponent const& other)
{
    m_color = other.m_color;
    m_intensity = other.m_intensity;
    m_radius = other.m_radius;

    return *this;
}

void PointLightComponent::Serialize(std::ostream& out) const
{
    out << "      \"PointLight\": {\n";

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

    data->m_color = { m_color, 0 };
    data->m_param = { m_intensity, m_radius, 0, 0 };
    data->m_position = { GetOwner()->GetGlobalTransform().getForward(), 0 };
}
